#ifndef __LOOM_SYSC_HPP__
#define __LOOM_SYSC_HPP__

#include "types.hpp"

namespace Loom {
namespace System {

    extern void LOOM_API Load();

    extern void LOOM_API exit(ubyte*);
    extern void LOOM_API printf(ubyte*);
    extern void LOOM_API strtol(ubyte*);
    extern void LOOM_API print_page(ubyte*);

    // deprecated
    extern void LOOM_API print_int(ubyte*);
    extern void LOOM_API print_hex(ubyte*);
    extern void LOOM_API print_string(ubyte*);
    extern void LOOM_API print_bool(ubyte*);
    extern void LOOM_API sqrt(ubyte*);
    extern void LOOM_API test_primes(ubyte*);

} }

#endif//#ifndef __LOOM_SYSC_HPP__
