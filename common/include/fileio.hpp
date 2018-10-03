#ifndef __LOOM_FILE_IO_HPP__
#define __LOOM_FILE_IO_HPP__

#include "code.hpp"
#include "types.hpp"

namespace Loom {

    extern std::string load_file(const char*);
    extern void save_file(const char*, uint32, const ubyte*);

}

#endif//__LOOM_FILE_IO_HPP__