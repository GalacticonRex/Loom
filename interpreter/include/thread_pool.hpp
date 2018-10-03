#ifndef __LOOM_THREAD_POOL_HPP__
#define __LOOM_THREAD_POOL_HPP__

#include "types.hpp"
#include "code.hpp"
#include "function_table.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>

namespace Loom {

    struct ThreadPool {

        struct Stack {

            enum class Return {
                SERIAL, // returns to the same stack
                ASYNC,  // returns to different stack
                SYNC,   // returns to different stack
                        // and needs to notify a SYNC structure
                KERN    // returns from a kernel call
            };

            static const char* returnTypeToString(Return);

        private:

            struct __buffer_req__ {
                ubyte access_type;
                Unit size;
            };
            
            std::unordered_map<void*, __buffer_req__> _buffers;
            std::vector<std::pair<Return, Unit>> _retn;
            ubyte* _base;
            Unit* _top;
            ubyte* _data;
            Unit* _frame;

        public:

            Unit sync;
            Unit config[3];
            Unit copy;

            // Kernel data
            ubyte const* kernel_instr;
            Unit index[3];
            Unit terminate[3];

            Stack(Unit init_size);
            ~Stack();

            void reset();

            ubyte* ptr() const;
            ubyte* base() const;

            void move(Unit);
            void push(Unit);
            void pop();

            bool addressStackEmpty() const;
            Unit addressStackSize() const;
            Unit addressStackTop() const;
            void pushAddress(Return, Unit);
            std::pair<Return, Unit> popAddress();

            void configureKernelCallForHost();
            void pushBuffer(ubyte access, Unit size, void* buffer, void* target);

        };

        struct Request {
            Stack::Return retn_type;
            Unit retn_sync;
            ubyte const* instruction;
            ubyte const* root;
            Unit data_size;
            ubyte* data;
            FunctionTable::Local ftable;
        };

        ThreadPool(std::size_t thread_count, std::size_t stack_size, std::size_t sync_count);
        ~ThreadPool();

        Unit threadCount() const;
        Unit backlogSize() const;
        Unit syncCount() const;
        Unit stackSize() const;

        bool backlogTooBig() const;

        CODE::Sync& getSync(Unit);
        Unit syncAlloc();
        void syncDealloc(Unit);

        bool active() const;
        void exec(const Request& req);
        bool retn(ubyte const*&, ubyte const*, Stack*&);

    private:

        static void __thread_main(ThreadPool*, std::size_t);
        void __thread_setup(std::size_t index,
                            ubyte const** instr,
                            ubyte const** root,
                            Stack** stack,
                            FunctionTable::Local* ftab,
                            std::mutex* block,
                            std::condition_variable* cond);
        void __task_done();
        void __run_next_task(std::size_t index);

        Stack* __get_stack();
        void __return_stack(Stack*);

        Unit _thread_max_count;
        Unit _sync_max_count;
        Unit _thread_stack_size;

        // Determines if the thread pool is currently working
        bool _active;

        // Make sure the pool doesn't start-up prematurely
        std::mutex _startup_block;
        std::condition_variable _startup_wait;
        std::atomic<Unit> _unavailable_threads;

        // Make sure the pool doesn't exit prematurely
        std::mutex _termin_block;
        std::condition_variable _termin_wait;
        std::atomic<Unit> _tasks_remaining;

        // Synchronization resources
        Unit _sync_allocated;
        CODE::Sync* _sync_resources;
        std::vector<Unit> _sync_available;

        // Stacks
        std::mutex _stack_acquisition_mutex;
        std::unordered_set<Stack*> _all_stacks;
        std::deque<Stack*> _available_stacks;

        // Values from threads
        std::vector<std::thread> _threads;
        std::vector<ubyte const**> _thread_instr;
        std::vector<ubyte const**> _thread_root;
        std::vector<Stack**> _thread_stack;
        std::vector<FunctionTable::Local*> _thread_ftable;
        std::vector<std::mutex*> _thread_mutex;
        std::vector<std::condition_variable*> _thread_cond;

        // Requests that still need to be processed
        std::queue<Request> _backlog;
        std::mutex _backlog_block;
        std::queue<std::size_t> _available_threads;

        // Stat resources
        #ifdef _STAT_THREADS_
        Unit _STAT_max_backlog_size;
        Unit _STAT_max_stacks_in_use;
        #endif//_STAT_THREADS_

    };

}

#endif//__LOOM_THREAD_POOL_HPP__
