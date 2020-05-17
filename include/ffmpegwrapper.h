#pragma once

#include "pch.h"
#include <cstdint>

namespace cszt0
{
    class FFmpegDecoderWrapperInterface
    {
    public:
        class PacketWrapperInterface
        {
        public:
            virtual ~PacketWrapperInterface() = default;

            /// check if this packet is from audio stream
            virtual bool IsAudioStream() const = 0;
            /// check if this packet is from video stream
            virtual bool IsVideoStream() const = 0;

            /// decode the packet, and resample if necessary
            /// return false when cannot decode or resample packet, which means
            /// you shouldn't call GetData later.
            virtual bool DecodeAndResample() = 0;
            /// get frame data. call DecodeAndResample first.
            ///
            /// if this frame is from audio stream, size means PCM sample count
            /// and only the first element of pData will be use. the length of pData[0]
            /// must be 4 times of size at least. return the number of samples actually obtained.
            /// if call this function many times, it will return the data behind last get. after
            /// all data has been got, it will just return 0 and do nothing.
            ///
            /// if this frame is from video stream, size will be ignore.
            /// the length of pData muse be 3 at least, and the elements of pData will be used to:
            ///     pData[0]: fill with Y, which need width * height space
            ///     pData[1]: fill with U, which need width * height / 4 space
            ///     pData[2]: fill with V, which need width * height / 4 space
            /// return value equals width * height
            /// if call this function many times, it will return the same data
            virtual int GetData(uint8_t **pData, int size) = 0;
        };

        virtual ~FFmpegDecoderWrapperInterface() = default;

        /// get the duration for the media (unit is ms)
        virtual int64_t GetDuration() const = 0;

        /// check if the media contains audio stream
        virtual bool ContainsAudioStream() const = 0;
        /// check if the media contains video stream
        virtual bool ContainsVideoStream() const = 0;

        /// if the media contains video, get the size of the video.
        /// in order to allow user use OpenGL to render the frame, width
        /// is always a multiple of 4
        virtual void GetVideoRect(int &width, int &height) const = 0;
        /// get FPS
        virtual float GetVideoFramePerSecond() const = 0;

        /// seek the stream to the time (unit is ms)
        virtual void SeekToTime(int64_t time) = 0;

        /// read the next frame and return its wrapper
        /// return null if no data.
        virtual PacketWrapperInterface *NextFrame() = 0;
    };

    /// create a new ffmpeg decoder wrapper
    extern FFAPI FFmpegDecoderWrapperInterface *FFCALL NewFFmpegDecoderWrapper(const char *filepath);
    /// delete a ffmpeg decoder wrapper
    extern FFAPI void FFCALL DeleteFFmpegDecoderWrapper(FFmpegDecoderWrapperInterface *wrapper);
} // namespace cszt0