#include "arguments.hpp"
#include "util.hpp"
#include "thread_pool.hpp"
#include "interpreter.hpp"

namespace Loom {

const char* ThreadPool::Stack::returnTypeToString(Return r) {
    switch(r) {
        case Return::SERIAL:
            return "Serial";
        case Return::ASYNC:
            return "ASync";
        case Return::SYNC:
            return "Sync";
        case Return::KERN:
            return "Kern";
        default:
            return "Unknown";
    }
}

ThreadPool::Stack::Stack(Unit init_size) :
    _base(new ubyte[init_size]),
    _top((Unit*)(_base + init_size)),
    _data(_base),
    _frame(_top),
    sync(0),
    config{0,0,0},
    copy(0)
    { ; }
ThreadPool::Stack::~Stack() {
    delete[] _base;
}

void ThreadPool::Stack::reset() {
    _data = _base;
    _frame = _top;
    sync = 0;
    config[0] = 0;
    config[1] = 0;
    config[2] = 0;    
    copy = 0;
}

ubyte* ThreadPool::Stack::ptr() const {
    return _data;
}
ubyte* ThreadPool::Stack::base() const {
    return _base;
}

void ThreadPool::Stack::move(Unit value) {
    _data += value;
}
void ThreadPool::Stack::push(Unit value) {
    this -> move(value);
    -- _frame;
    *_frame = value;
}
void ThreadPool::Stack::pop() {
    ubyte* value = _data - *_frame;
    if ( value < _base )
        sendError("Cannot pop off of an empty stack");
    _data = value;
    ++ _frame;
}
bool ThreadPool::Stack::addressStackEmpty() const {
    return _retn.empty();
}
Unit ThreadPool::Stack::addressStackSize() const {
    return _retn.size();
}
Unit ThreadPool::Stack::addressStackTop() const {
    return *_frame;
}
void ThreadPool::Stack::pushAddress(Return r, Unit ad) {
    _retn.push_back(std::make_pair(r, ad));
}
std::pair<ThreadPool::Stack::Return, Unit>
ThreadPool::Stack::popAddress() {
    if ( _retn.empty() ) {
        sendError("No calling function to return to");
    }
    std::pair<Return, Unit> result = _retn.back();
    _retn.pop_back();
    return result;
}

void ThreadPool::Stack::pushBuffer(ubyte access, Unit size, void* buffer, void* target) {
    std::unordered_map<void*, __buffer_req__>::iterator iter = 
        _buffers.find(target);
    if ( iter != _buffers.end() )
        sendError("Attempt to double buffer a location");

    __buffer_req__ breq{access, size};
    _buffers.insert(std::make_pair(target, breq));
    * (void**) target = buffer;
}

}
