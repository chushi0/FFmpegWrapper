# FFMpegWrapper
A library wrap FFmpeg.

## Compile
Firstly, you must compile FFmpeg library first. [Click here](https://github.com/ffmpeg/ffmpeg) to go to their repository.

Secondly, you must install cmake and another c++ compiler.

Then, use the following command to compile this library. Note that replace `<ffmpeg-include-dir>` and `<ffmpeg-library-dir>`
to your path.
```shell
mkdir build
cd build
cmake -DFFMPEG_INCLUDE_DIR=<ffmpeg-include-dir> -DFFMPEG_LIBRARY_DIR=<ffmpeg-library-dir> ..
make
```
The finally command means that use your c++ compiler to compile the code.

Finally, copy `libffmpegwrapper.so` or `ffmpegwrapper.lib` or others to your own project LIB path,
and copy `include` dir to your own project INCLUDE path.

## Usage
include `include "ffmpegwrapper.h"`, call function `cszt0::NewFFmpegDecoderWrapper` to create a decoder wrapper,
then you can call its member functions to use it.
