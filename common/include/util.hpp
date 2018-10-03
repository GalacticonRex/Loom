#ifndef __LOOM_UTIL_HPP__
#define __LOOM_UTIL_HPP__

#include <stdarg.h>
#include "types.hpp"

namespace Loom {

    extern void sendError(const char*, ...);
    extern uint64 getThreadId();

}

#endif//__LOOM_UTIL_HPP__