#pragma once

#include "include/ffmpegwrapper.h"
#include "ffmpeg.h"

namespace cszt0
{
    class FFmpegDecoderWrapper : public FFmpegDecoderWrapperInterface
    {
    public:
        class PacketWrapper : public PacketWrapperInterface
        {
        public:
            virtual bool IsAudioStream() const override;
            virtual bool IsVideoStream() const override;

            virtual bool DecodeAndResample() override;
            virtual int GetData(uint8_t **pData, int size) override;

        private:
            FFmpegDecoderWrapper *ffmpeg;

            friend class FFmpegDecoderWrapper;
        };

        FFmpegDecoderWrapper(AVFormatContext *pAVFormatContext);
        FFmpegDecoderWrapper(const FFmpegDecoderWrapper &) = delete;
        FFmpegDecoderWrapper &operator=(const FFmpegDecoderWrapper &) = delete;

        virtual ~FFmpegDecoderWrapper();

        virtual int64_t GetDuration() const override;

        virtual bool ContainsAudioStream() const override;
        virtual bool ContainsVideoStream() const override;

        virtual void GetVideoRect(int &width, int &height) const override;
        virtual float GetVideoFramePerSecond() const override;

        virtual void SeekToTime(int64_t time) override;

        virtual PacketWrapperInterface *NextFrame() override;

    private:
        AVFormatContext *pAVFormatContext;

        AVStream *pAudioStream;
        AVStream *pVideoStream;

        int audioStreamIndex;
        int videoStreamIndex;

        AVCodecContext *pAudioCodecContext;
        AVCodecContext *pVideoCodecContext;

        AVCodec *pAudioCodec;
        AVCodec *pVideoCodec;

        AVFrame *pAVFrame;
        AVPacket *pAVPacket;
        SwrContext *pSwrContext;
        bool containsFrame;
        bool containsPacket;
        PacketWrapper packet;

        void FindAndSetStream(AVMediaType streamType, AVStream *&stream, int &streamIndex, AVCodecContext *&codecContext, AVCodec *&codec);

        friend class PacketWrapper;
    };
} // namespace cszt0