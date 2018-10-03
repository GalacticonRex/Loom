#include "util.hpp"
#include <cstdio>
#include <string>
#include <thread>

namespace Loom {

    void sendError(const char* fmt, ...) {
        char buffer[1024];
        va_list args;
        va_start (args, fmt);
        vsprintf (buffer,fmt, args);
        va_end (args);

        throw std::string(buffer);
    }

    extern uint64 getThreadId() {
        return (uint64)std::hash<std::thread::id>{}(std::this_thread::get_id());
    }

}