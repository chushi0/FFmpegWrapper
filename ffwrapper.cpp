#include "include/ffmpegwrapper.h"
#include "ffdecoder.h"

namespace cszt0
{
    FFAPI FFmpegDecoderWrapperInterface *FFCALL NewFFmpegWrapper(const char *filepath)
    {
        AVFormatContext *pAVFormatContext = avformat_alloc_context();

        // 打开文件
        if (avformat_open_input(&pAVFormatContext, filepath, nullptr, nullptr) < 0 || avformat_find_stream_info(pAVFormatContext, nullptr) < 0)
        {
            avformat_free_context(pAVFormatContext);
            return nullptr;
        }

        return new FFmpegDecoderWrapper(pAVFormatContext);
    }

    FFAPI void FFCALL DeleteFFmpegWrapper(FFmpegDecoderWrapperInterface *ffmpeg)
    {
        delete ffmpeg;
    }
} // namespace cszt0