#include "memory_pool.hpp"
#include "util.hpp"
#include <iostream>
#include <iomanip>

namespace Loom {

void MemoryPool::printPage(std::ostream& out, ubyte* dat, uint32 w, uint32 h) {
    std::ios::fmtflags fl( std::cerr.flags() );
    std::cerr << " --- POINTER " << (void*)dat << " --- " << std::endl;
    for ( uint32 row=0;row<h;row++ ) {
        for ( uint32 col=0;col<w;col++ )
            out << std::setw(2)
                << std::setfill('0')
                << std::hex
                << (uint32)dat[col+row*w]
                << " ";
        out << std::endl;
    }
    std::cerr.flags( fl );
}

MemoryPool::MemoryPool(Unit bytes) :
    _data_byte_size(bytes),
    _data(new Code[bytes]),
    _page_size( 1<<(8*sizeof(Code)) ) {
    _available_pages.resize(_data_byte_size / _page_size);
    for ( uint32 i=0;i<_available_pages.size();i++ ) {
        _available_pages[i] = i;
    }
}
MemoryPool::~MemoryPool() {
    delete[] _data;
}

Code* MemoryPool::alloc() {
    if ( _available_pages.empty() )
        sendError("Out of memory");

    uint32 top = _available_pages.back();
    _available_pages.pop_back();

    Code* result = _data + _page_size * top;
    
    return result;
}
void MemoryPool::dealloc(Code* b) {
    if ( b < _data || b >= _data + _data_byte_size)
        sendError("Invalid memory address");

    Unit dat = (Unit)(b - _data) / _page_size;

    //std::cerr << "dealloc " << (void*)b << " = " << seg << " : " << dat << std::endl;

    _available_pages.push_back(dat);
}

}