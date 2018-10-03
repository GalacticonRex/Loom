#ifndef __LOOM_DEFINE_TYPES_HPP__
#define __LOOM_DEFINE_TYPES_HPP__

#include <cstdint>

#ifdef _CYGWIN_BUILD_
    #ifdef _EXPORTING_
        #define LOOM_API __declspec(dllexport)
    #else
        #define LOOM_API __declspec(dllimport)
    #endif
#else
    #define LOOM_API
#endif

typedef int8_t   byte;
typedef uint8_t  ubyte;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef float    float32;
typedef double   float64;

#endif//__DEFINE_TYPES_HPP__
