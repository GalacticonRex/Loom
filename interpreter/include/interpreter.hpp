#ifndef __LOOM_INTERPRETER_HPP__
#define __LOOM_INTERPRETER_HPP__

#include "types.hpp"
#include "thread_pool.hpp"

namespace Loom {

    //======================================================================
    //
    // CLASS : Interpreter
    // 
    //
    //======================================================================

    struct Interpreter {
    private:

        ThreadPool& _pool;

    public:

        Interpreter(ThreadPool&);
        ~Interpreter();

        void exec(ubyte const*& instr,
                  const ubyte* base,
                  ThreadPool::Stack*& stack,
                  ThreadPool* pool,
                  FunctionTable::Local ftab);

        static void runProgram(ThreadPool&,
                               FunctionTable&,
                               Code* program, 
                               int argc,
                               char** argv);

    };

}

#endif//__LOOM_INTERPRETER_HPP__