#ifndef __LOOM_MEMORY_POOL_HPP__
#define __LOOM_MEMORY_POOL_HPP__

#include <vector>
#include <mutex>
#include <ostream>
#include "types.hpp"
#include "code.hpp"

namespace Loom {

    struct MemoryPool {

        typedef uint32 ThreadIndex;

    private:

        Unit _data_byte_size;
        Code* _data;

        Unit _page_size;
        std::vector<Unit> _available_pages;

    public:

        static void printPage(std::ostream&, ubyte*, uint32, uint32);

        MemoryPool(Unit bytes);
        ~MemoryPool();

        Code* alloc();
        void dealloc(Code*);

    };

}

#endif//__LOOM_MEMORY_POOL_HPP__