#include "ffdecoder.h"

namespace cszt0
{
    FFmpegDecoderWrapper::FFmpegDecoderWrapper(AVFormatContext *pAVFormatContext) : pAVFormatContext(pAVFormatContext),
                                                                                    pAudioStream(nullptr),
                                                                                    pVideoStream(nullptr),
                                                                                    audioStreamIndex(-1),
                                                                                    videoStreamIndex(-1),
                                                                                    pAudioCodecContext(nullptr),
                                                                                    pVideoCodecContext(nullptr),
                                                                                    pAudioCodec(nullptr),
                                                                                    pVideoCodec(nullptr),
                                                                                    containsFrame(false),
                                                                                    containsPacket(false)
    {
        packet.ffmpeg = this;

        FindAndSetStream(AVMEDIA_TYPE_AUDIO, pAudioStream, audioStreamIndex, pAudioCodecContext, pAudioCodec);
        FindAndSetStream(AVMEDIA_TYPE_VIDEO, pVideoStream, videoStreamIndex, pVideoCodecContext, pVideoCodec);

        pSwrContext = swr_alloc();
        if (ContainsAudioStream())
        {
            // 音频重采样为：
            // 采样率	44100
            // 双声道
            // 16位带符号
            swr_alloc_set_opts(pSwrContext, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
                               // pAudioCodec->channel_layouts ? *pAudioCodec->channel_layouts :
                               pAudioCodecContext->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO, *pAudioCodec->sample_fmts, pAudioCodecContext->sample_rate, 0, 0);
            swr_init(pSwrContext);
        }

        pAVFrame = av_frame_alloc();
        pAVPacket = av_packet_alloc();
    }

    FFmpegDecoderWrapper::~FFmpegDecoderWrapper()
    {
        if (containsFrame)
        {
            av_frame_unref(pAVFrame);
            containsFrame = false;
        }
        av_frame_free(&pAVFrame);
        if (containsPacket)
        {
            av_packet_unref(pAVPacket);
            containsPacket = false;
        }
        av_packet_free(&pAVPacket);
        avformat_free_context(pAVFormatContext);

        swr_free(&pSwrContext);
    }

    int64_t FFmpegDecoderWrapper::GetDuration() const
    {
        if (pAVFormatContext->duration != AV_NOPTS_VALUE)
        {
            return pAVFormatContext->duration * 1000 / AV_TIME_BASE;
        }
        return 0;
    }

    bool FFmpegDecoderWrapper::ContainsAudioStream() const
    {
        return audioStreamIndex != -1;
    }

    bool FFmpegDecoderWrapper::ContainsVideoStream() const
    {
        return videoStreamIndex != -1;
    }

    void FFmpegDecoderWrapper::GetVideoRect(int &width, int &height) const
    {
        if (pVideoCodecContext)
        {
            width = pVideoCodecContext->width;
            height = pVideoCodecContext->height;
            // 减去 1~7 像素，保证宽度是 8 的倍数
            // 这会在 OpenGL 上用到，同时不会影响观看体验
            width -= width % 8;
        }
    }

    float FFmpegDecoderWrapper::GetVideoFramePerSecond() const
    {
        if (pVideoStream)
        {
            if ((pVideoStream->avg_frame_rate.num <= 0) || (pVideoStream->avg_frame_rate.den <= 0))
            {
                return 25;
            }
            else
            {
                return (float)pVideoStream->avg_frame_rate.num / (float)pVideoStream->avg_frame_rate.den;
            }
        }
        return 0;
    }

    void FFmpegDecoderWrapper::SeekToTime(int64_t time)
    {

        int status = av_seek_frame(pAVFormatContext, -1, time * AV_TIME_BASE / 1000, AVSEEK_FLAG_BACKWARD);
        if (pAudioCodecContext)
            avcodec_flush_buffers(pAudioCodecContext);
        if (pVideoCodecContext)
            avcodec_flush_buffers(pVideoCodecContext);
    }

    FFmpegDecoderWrapperInterface::PacketWrapperInterface *FFmpegDecoderWrapper::NextFrame()
    {
        if (containsFrame)
        {
            av_frame_unref(pAVFrame);
        }
        if (containsPacket)
        {
            av_packet_unref(pAVPacket);
        }
        if (av_read_frame(pAVFormatContext, pAVPacket))
        {
            return nullptr;
        }
        containsPacket = true;
        return &packet;
    }

    void FFmpegDecoderWrapper::FindAndSetStream(AVMediaType streamType, AVStream *&stream, int &streamIndex, AVCodecContext *&codecContext, AVCodec *&codec)
    {
        for (unsigned int i = 0; i < pAVFormatContext->nb_streams; i++)
        {
            if (pAVFormatContext->streams[i]->codec->codec_type == streamType)
            {
                streamIndex = i;
                break;
            }
        }
        if (streamIndex != -1)
        {
            stream = pAVFormatContext->streams[streamIndex];
            codecContext = stream->codec;
            codec = avcodec_find_decoder(codecContext->codec_id);
            if (!codec)
            {
                streamIndex = -1;
                stream = nullptr;
                codecContext = nullptr;
            }
            if (avcodec_open2(codecContext, codec, nullptr) < 0)
            {
                streamIndex = -1;
                stream = nullptr;
                codecContext = nullptr;
            }
        }
    }

    bool FFmpegDecoderWrapper::PacketWrapper::IsAudioStream() const
    {
        return ffmpeg->pAVPacket->stream_index == ffmpeg->audioStreamIndex;
    }

    bool FFmpegDecoderWrapper::PacketWrapper::IsVideoStream() const
    {
        return ffmpeg->pAVPacket->stream_index == ffmpeg->videoStreamIndex;
    }

    bool FFmpegDecoderWrapper::PacketWrapper::DecodeAndResample()
    {
        int status;
        if (IsAudioStream())
        {
            status = avcodec_send_packet(ffmpeg->pAudioCodecContext, ffmpeg->pAVPacket);
            if (status)
                return false;
            status = avcodec_receive_frame(ffmpeg->pAudioCodecContext, ffmpeg->pAVFrame);
            if (status)
                return false;
            status = swr_convert(ffmpeg->pSwrContext, nullptr, 0, (const uint8_t **)(ffmpeg->pAVFrame->data), ffmpeg->pAVFrame->nb_samples);
        }
        else
        {
            status = avcodec_send_packet(ffmpeg->pVideoCodecContext, ffmpeg->pAVPacket);
            if (status)
                return false;
            status = avcodec_receive_frame(ffmpeg->pVideoCodecContext, ffmpeg->pAVFrame);
            if (status)
                return false;
        }
        ffmpeg->containsFrame = true;
        return true;
    }

    int FFmpegDecoderWrapper::PacketWrapper::GetData(uint8_t **pData, int size)
    {
        if (IsAudioStream())
        {
            return swr_convert(ffmpeg->pSwrContext, pData, size, nullptr, 0);
        }
        else
        {
            int width, height;
            ffmpeg->GetVideoRect(width, height);
            int fullSize = width * height;
            int halfWidth = width >> 1;
            int halfHeight = height >> 1;
            for (int i = 0; i < height; i++)
            {
                memcpy(pData[0] + width * i, ffmpeg->pAVFrame->data[0] + ffmpeg->pAVFrame->linesize[0] * i, width);
            }
            for (int i = 0; i < halfHeight; i++)
            {
                memcpy(pData[1] + halfWidth * i, ffmpeg->pAVFrame->data[1] + ffmpeg->pAVFrame->linesize[1] * i, halfWidth);
                memcpy(pData[2] + halfWidth * i, ffmpeg->pAVFrame->data[2] + ffmpeg->pAVFrame->linesize[2] * i, halfWidth);
            }
            return fullSize;
        }
    }
} // namespace cszt0