#include "arguments.hpp"
#include "util.hpp"
#include "thread_pool.hpp"
#include "interpreter.hpp"

namespace Loom {

ThreadPool::ThreadPool(std::size_t threadCount, std::size_t stackSize, std::size_t syncCount) :
    _thread_max_count(threadCount),
    _sync_max_count(syncCount),
    _thread_stack_size(stackSize),
    _active(true),
    _unavailable_threads(_thread_max_count),
    _tasks_remaining(0),
    _sync_allocated(0),
    _sync_resources(new CODE::Sync[_sync_max_count]),
    _threads(_thread_max_count),
    _thread_instr(_thread_max_count, nullptr),
    _thread_root(_thread_max_count, nullptr),
    _thread_stack(_thread_max_count, nullptr),
    _thread_ftable(_thread_max_count, nullptr),
    _thread_mutex(_thread_max_count),
    _thread_cond(_thread_max_count)
#ifdef _STAT_THREADS_
    , _STAT_max_backlog_size(0)
    , _STAT_max_stacks_in_use(0)
#endif
    {
    for ( std::size_t i=0;i<_thread_max_count;i++ ) {
        _threads[i] = std::thread(__thread_main, this, i);
        _available_threads.push(i);
    }

    std::unique_lock<std::mutex> lk_startup(_startup_block);
    while ( _unavailable_threads != 0 )
        _startup_wait.wait(lk_startup);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [+ Thread pool ready to receive programs ]\n");
    #endif
}
    
ThreadPool::~ThreadPool() {
    // Make sure there are no tasks left
    if ( _tasks_remaining != 0 ) {
        std::unique_lock<std::mutex> lk(_termin_block);
        while ( _tasks_remaining != 0 )
            _termin_wait.wait(lk);
    }

    // Set active to false so we accept no new tasks
    _active = false;

    // Join all threads
    for ( Unit i=0;i<_threads.size();i++ ) {
        {
            std::lock_guard<std::mutex> lk(*_thread_mutex[i]);
            _thread_cond[i] -> notify_all();
        }
        _threads[i].join();
        delete _thread_mutex[i];
        delete _thread_cond[i];
    }

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [+ Delete synchronization resources ]\n");
    #endif

    // Delete our sync resource pool
    delete[] _sync_resources;

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [+ Delete stack resources ]\n");
    #endif

    for ( std::unordered_set<Stack*>::iterator i=_all_stacks.begin();
          i != _all_stacks.end();i++ ) {
        delete *i;
    }

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [+ Thread pool exitting ]\n");
    #endif

    #ifdef _STAT_THREADS_
    fprintf(stderr,
        "==========\n"
        "Thread Pool Statistics:\n"
        "Max Backlog Size: %lu\n"
        "Max Stack Use:    %lu\n"
        "==========\n",
        _STAT_max_backlog_size,
        _STAT_max_stacks_in_use
        );
    #endif
}

Unit ThreadPool::threadCount() const {
    return _thread_max_count;
}
Unit ThreadPool::backlogSize() const {
    return _backlog.size();
}
Unit ThreadPool::syncCount() const {
    return _sync_max_count;
}
Unit ThreadPool::stackSize() const {
    return _thread_stack_size;
}

bool ThreadPool::backlogTooBig() const {
    return ( backlogSize() > 2 * threadCount() );
}

CODE::Sync& ThreadPool::getSync(Unit sync) {
    // Make sure we're not trying to allocate an invalid object
    if ( sync >= _sync_max_count )
        sendError("Attempting to get sync resource %d, only have %d available",
            sync,
            _sync_max_count
            );

    return _sync_resources[sync];
}
Unit ThreadPool::syncAlloc() {
    Unit result;
    
    // if there is no available object, allocate a new one from the top
    if ( _sync_available.empty() ) {
        if ( _sync_allocated >= _sync_max_count )
            sendError("There are no sync objects left to allocate!");
        result = _sync_allocated;
        ++ _sync_allocated;
    }
    
    // otherwise get the top one off the waiting stack
    else {
        result = _sync_available.back();
        _sync_available.pop_back();
    }

    return result;
}
void ThreadPool::syncDealloc(Unit sync) {
    // make sure that this object has at one point been allocated
    if ( sync >= _sync_allocated )
        sendError("Invalid sync resource %d", sync);

    // NOTE: doesn't check for double deallocation
    _sync_available.push_back(sync);
}

bool ThreadPool::active() const {
    return _active;
}
void ThreadPool::exec(const Request& req) {
    std::unique_lock<std::mutex> lk(_backlog_block);

     #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [# received execution request ]\n");
    #endif//_DEBUG_ADVANCED_

    // increment the number of tasks
    ++ _tasks_remaining;

     #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [# there are now %lu tasks left to complete ]\n", _tasks_remaining.load() );
    #endif//_DEBUG_ADVANCED_
    
    // if there are no available threads put this request into the backlog
    if ( _available_threads.empty() ) {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [# no available threads, moving to backlog ]\n" );
        #endif//_DEBUG_ADVANCED_

        _backlog.push(req);
        #ifdef _STAT_THREADS_
        if ( _backlog.size() > _STAT_max_backlog_size ) {
            _STAT_max_backlog_size = _backlog.size();
        }
        #endif
    }

    // otherwise immediately notify the next waiting thread
    else {

        std::size_t i = _available_threads.front();
        _available_threads.pop();

        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [# execute request now on thread %lu ]\n", i);
        #endif//_DEBUG_ADVANCED_

        std::lock_guard<std::mutex> lk_block(*_thread_mutex[i]);

        *_thread_instr[i] = req.instruction;
        *_thread_root[i] = req.root;
        *_thread_stack[i] = __get_stack();
        *_thread_ftable[i] = req.ftable;

        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [# return type is %s ]\n", Stack::returnTypeToString(req.retn_type));
        #endif//_DEBUG_ADVANCED_

        if ( req.retn_type == Stack::Return::KERN ) {

            Unit* kern_values = (Unit*)req.data;

            //Unit kern_config = kern_values[0];

            #ifdef _DEBUG_ADVANCED_
            fprintf(stderr, "    [# kernel lvl1 = (%lu .. %lu) ]\n", kern_values[1], kern_values[2]);
            #endif//_DEBUG_ADVANCED_

            (*_thread_stack[i]) -> kernel_instr = req.instruction;

            (*_thread_stack[i]) -> index[0] = kern_values[1];
            (*_thread_stack[i]) -> terminate[0] = kern_values[2];
            (*_thread_stack[i]) -> index[1] = 0;
            (*_thread_stack[i]) -> terminate[1] = 0;
            (*_thread_stack[i]) -> index[2] = 0;
            (*_thread_stack[i]) -> terminate[2] = 0;

            #ifdef _DEBUG_ADVANCED_
            fprintf(stderr, "    [# copy %lu bytes to stack ]\n", req.data_size - sizeof(Unit[3]));
            #endif//_DEBUG_ADVANCED_

            ubyte* ptr = (*_thread_stack[i]) -> ptr();
            std::copy(req.data + sizeof(Unit[3]), req.data + req.data_size, ptr);
            delete[] req.data;

            (*_thread_stack[i]) -> move(req.data_size);

        } else {

            #ifdef _DEBUG_ADVANCED_
            fprintf(stderr, "    [# copy %lu bytes to stack ]\n", req.data_size);
            #endif//_DEBUG_ADVANCED_

            // copy request data to the new stack
            ubyte* ptr = (*_thread_stack[i]) -> ptr();
            std::copy(req.data, req.data + req.data_size, ptr);
            delete[] req.data;

        }

        (*_thread_stack[i]) -> pushAddress(req.retn_type, req.retn_sync);
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "Push %p(%lu) on %lu\n", *_thread_stack[i], (*_thread_stack[i]) -> addressStackSize(), getThreadId());
        #endif

        _thread_cond[i] -> notify_all();
    }
}

bool ThreadPool::retn(ubyte const*& instr, ubyte const* root, Stack*& stack) {
    bool result = false;

    #ifdef _DEBUG_SIMPLE_
    fprintf(stderr, "Pop %p(%lu) on %lu\n", stack, stack -> addressStackSize(), getThreadId());
    #endif

    std::pair<Stack::Return, Unit> addr =
        stack -> popAddress();
    switch ( addr.first ) {
        case Stack::Return::SERIAL:

            #ifdef _DEBUG_SIMPLE_
            fprintf(stderr, "    [# return from serial call (old=%lu, new=%lu) ]\n", (Unit)(instr - root), addr.second);
            #endif//_DEBUG_SIMPLE_

            if ( stack -> addressStackEmpty() ) {
                this -> __return_stack(stack);
                result = false;
            } else {
                instr = root + addr.second;
                result = true;
            }

            break;
        case Stack::Return::ASYNC:

            #ifdef _DEBUG_SIMPLE_
            fprintf(stderr, "    [# return from async call ]\n");
            #endif//_DEBUG_SIMPLE_

            this -> __return_stack(stack);
            result = false;

            break;
        case Stack::Return::SYNC: {

            #ifdef _DEBUG_SIMPLE_
            fprintf(stderr, "    [# return from sync call (sync=%lu) ]\n", addr.second);
            #endif//_DEBUG_SIMPLE_

            this -> __return_stack(stack);

            Unit sync_loc = addr.second;
            CODE::Sync& sync = this -> getSync(sync_loc);
            std::lock_guard<std::mutex> lk(sync.getLock());

            (void) sync.removeDep();

            #ifdef _DEBUG_SIMPLE_
            fprintf(stderr, "    [# has %lu deps ]\n", sync.getDep());
            fprintf(stderr, "    [# return address = %p ]\n", (void*) sync.getTarget());
            fprintf(stderr, "    [# return stack = %p ]\n", (void*) sync.getStack());
            #endif//_DEBUG_SIMPLE_

            if ( sync.getDep() == 0 && sync.getStack() != 0 ) {
                #ifdef _DEBUG_SIMPLE_
                fprintf(stderr, "    [# all dependecies are complete, continue from STOP (instr=%p, stack=%p) ]\n", instr, stack);
                #endif//_DEBUG_SIMPLE_

                instr = (ubyte const*) sync.getTarget();
                stack = (Stack*) sync.getStack();

                this -> syncDealloc(sync_loc);
                result = true;
            } else {
                result = false;
            }

        }   break;
        case Stack::Return::KERN:

            #ifdef _DEBUG_SIMPLE_
            fprintf(stderr, "    [# return from kernel call (sync=%lu) ]\n", addr.second);
            #endif//_DEBUG_SIMPLE_
            
            if ( ++ stack -> index[0] >= stack -> terminate[0] ) {

                #ifdef _DEBUG_ADVANCED_
                fprintf(stderr, "    [# reset index 1 ]\n");
                #endif//_DEBUG_ADVANCED_

                if ( ++ stack -> index[1] >= stack -> terminate[1] ) {

                    #ifdef _DEBUG_ADVANCED_
                    fprintf(stderr, "    [# reset index 2 ]\n");
                    #endif//_DEBUG_ADVANCED_

                    if ( ++ stack -> index[2] >= stack -> terminate[2] ) {

                        #ifdef _DEBUG_ADVANCED_
                        fprintf(stderr, "    [# reset index 3 ]\n");
                        #endif//_DEBUG_ADVANCED_

                        this -> __return_stack(stack);

                        Unit sync_loc = addr.second;
                        CODE::Sync& sync = this -> getSync(sync_loc);
                        std::lock_guard<std::mutex> lk(sync.getLock());

                        (void) sync.removeDep();

                        if ( sync.getDep() == 0 && sync.getStack() != 0 ) {
                            #ifdef _DEBUG_SIMPLE_
                            fprintf(stderr, "    [# all dependecies are complete, continue from STOP (instr=%p, stack=%p) ]\n", instr, stack);
                            #endif//_DEBUG_SIMPLE_

                            instr = (ubyte const*) sync.getTarget();
                            stack = (Stack*) sync.getStack();

                            this -> syncDealloc(sync_loc);
                            result = true;
                        } else {
                            result = false;
                        }
                        break;
                    }
                    stack -> index[1] = 0;
                }
                stack -> index[0] = 0;
            }

            #ifdef _DEBUG_SIMPLE_
            fprintf(stderr, "    [# proceed to next kernel iteration (%lu, %lu, %lu) ]\n", stack -> index[0], stack -> index[1], stack -> index[2]);
            #endif//_DEBUG_SIMPLE_
            stack -> pushAddress(Stack::Return::KERN, addr.second);
            instr = stack -> kernel_instr;
            result = true;

            break;
        default:
            sendError("Unknown return code %d", addr.first);
    }

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [# return complete, %scontinue ]\n", ((result)?"":"do not "));
    #endif//_DEBUG_ADVANCED_

    return result;
}

// setup a particular thread at [index]
void ThreadPool::__thread_setup(std::size_t index,
                                ubyte const** instr,
                                ubyte const** root,
                                Stack** stack,
                                FunctionTable::Local* ftab,
                                std::mutex* block,
                                std::condition_variable* cond) {
    _thread_instr[index] = instr;
    _thread_root[index] = root;
    _thread_stack[index] = stack;
    _thread_ftable[index] = ftab;

    _thread_mutex[index] = block;
    _thread_cond[index] = cond;

    // if all the threads are ready to go, alert the pool
    _unavailable_threads -- ;
    if ( _unavailable_threads == 0 ) {
        std::lock_guard<std::mutex> lk_temp(_startup_block);
        _startup_wait.notify_all();
    }
}

void ThreadPool::__task_done() {
    if ( -- _tasks_remaining == 0 ) {
        std::lock_guard<std::mutex> lkt(_termin_block);
        _termin_wait.notify_all();
    }
}
void ThreadPool::__run_next_task(std::size_t i) {
    std::lock_guard<std::mutex> lk(_backlog_block);
    if ( _backlog.empty() ) {

        #ifdef _DEBUG_SIMPLE_
        fprintf(stderr, "    [# no available task, sleeping thread ]\n" );
        #endif//_DEBUG_ADVANCED_

        *(_thread_instr[i]) = nullptr;
        _available_threads.push(i);
    } else {

        #ifdef _DEBUG_SIMPLE_
        fprintf(stderr, "    [# getting next task from the backlog ]\n" );
        #endif//_DEBUG_ADVANCED_

        Request& req = _backlog.front();
        *(_thread_instr[i]) = req.instruction;
        *(_thread_root[i]) = req.root;
        *(_thread_stack[i]) = __get_stack();
        *(_thread_ftable[i]) = req.ftable;

        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [# return type is %s ]\n", Stack::returnTypeToString(req.retn_type));
        #endif//_DEBUG_ADVANCED_

        if ( req.retn_type == Stack::Return::KERN ) {

            Unit* kern_values = (Unit*)req.data;

            //Unit kern_config = kern_values[0];

            #ifdef _DEBUG_ADVANCED_
            fprintf(stderr, "    [# kernel lvl1 = (%lu .. %lu) ]\n", kern_values[1], kern_values[2]);
            #endif//_DEBUG_ADVANCED_

            (*_thread_stack[i]) -> kernel_instr = req.instruction;

            (*_thread_stack[i]) -> index[0] = kern_values[1];
            (*_thread_stack[i]) -> terminate[0] = kern_values[2];
            (*_thread_stack[i]) -> index[1] = 0;
            (*_thread_stack[i]) -> terminate[1] = 0;
            (*_thread_stack[i]) -> index[2] = 0;
            (*_thread_stack[i]) -> terminate[2] = 0;

            #ifdef _DEBUG_ADVANCED_
            fprintf(stderr, "    [# copy %lu bytes to stack ]\n", req.data_size - sizeof(Unit[3]));
            #endif//_DEBUG_ADVANCED_

            ubyte* ptr = (*_thread_stack[i]) -> ptr();
            std::copy(req.data + sizeof(Unit[3]), req.data + req.data_size, ptr);
            delete[] req.data;

            (*_thread_stack[i]) -> move(req.data_size);

        } else {

            #ifdef _DEBUG_ADVANCED_
            fprintf(stderr, "    [# copy %lu bytes to stack ]\n", req.data_size);
            #endif//_DEBUG_ADVANCED_

            // copy request data to the new stack
            ubyte* ptr = (*_thread_stack[i]) -> ptr();
            std::copy(req.data, req.data + req.data_size, ptr);
            delete[] req.data;

        }

        (*_thread_stack[i]) -> pushAddress(req.retn_type, req.retn_sync);
        #ifdef _DEBUG_SIMPLE_
        fprintf(stderr, "Push %p(%lu) on %lu\n", *_thread_stack[i], (*_thread_stack[i]) -> addressStackSize(), getThreadId());
        #endif

        _backlog.pop();
    }
}
ThreadPool::Stack* ThreadPool::__get_stack() {
    std::lock_guard<std::mutex> lk(_stack_acquisition_mutex);

    Stack* result = nullptr;

    if ( _available_stacks.empty() ) {
        result = new Stack(this -> stackSize());
        _all_stacks.insert(result);
        #ifdef _STAT_THREADS_
        if ( _all_stacks.size() > _STAT_max_stacks_in_use ) {
            _STAT_max_stacks_in_use = _all_stacks.size();
        }
        #endif
    } else {
        result = _available_stacks.front();
        result -> reset();
        _available_stacks.pop_front();
    }

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [# GET STACK %p ]\n", result);
    #endif

    return result;
}
void ThreadPool::__return_stack(Stack* st) {
    std::lock_guard<std::mutex> lk(_stack_acquisition_mutex);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [# RETN STACK %p ]\n", st);
    #endif

    if ( _available_stacks.size() >= _thread_max_count ) {
        delete st;
        _all_stacks.erase(st);
    } else
        _available_stacks.push_back(st);
}

void ThreadPool::__thread_main(ThreadPool* pool, std::size_t index) {
    Unit count = 0;
    ubyte const* instruction = nullptr;
    ubyte const* root = nullptr;
    Stack* stack = nullptr;
    FunctionTable::Local ftab = nullptr;

    std::mutex* block = new std::mutex();
    std::condition_variable* cond = new std::condition_variable();

    Interpreter local(*pool);

    std::unique_lock<std::mutex> lk(*block);
    pool -> __thread_setup(index, &instruction, &root, &stack, &ftab, block, cond);

    while ( pool -> active() ) {
        while ( !instruction ) {
            cond -> wait(lk);
            if ( !pool -> active() )
                goto __thread_main_exit;
        }

        try {
            #ifdef _DEBUG_SIMPLE_
            fprintf(stderr, "    [# START THREAD(%zu):(%lu) ]\n", index, count);
            fprintf(stderr, "Thread %lu uses stack %p\n", index, stack);
            #endif
            local.exec(instruction, root, stack, pool, ftab);
            #ifdef _DEBUG_SIMPLE_
            fprintf(stderr, "Thread %lu done with stack %p\n", index, stack);
            fprintf(stderr, "    [# END THREAD(%zu):(%lu) ]\n", index, count);
            #endif
        } catch ( std::string e ) {
            #ifdef _DEBUG_SIMPLE_
            fprintf(stderr, "    [# ERROR THREAD(%zu):(%lu) ]\n", index, count);
            #endif
            fprintf(stderr, "Error on thread %zu(%lu) | %s\n", index, getThreadId(), e.c_str());
            pool -> _active = false;
            pool -> __task_done();
            goto __thread_main_exit;
        }

        ++ count;

        pool -> __task_done();
        pool -> __run_next_task(index);
    }

__thread_main_exit:
    return;
}

}
