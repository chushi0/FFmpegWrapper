#pragma once

#if defined(_WIN32) || defined(_WIN64)
#ifdef FFMPEGWRAPPER_BUILD
#define FFAPI __declspec(dllexport)
#else
#define FFAPI __declspec(dllimport)
#endif
#define FFCALL __stdcall
#else
#define FFAPI
#define FFCALL __attribute__((__stdcall__))
#endif