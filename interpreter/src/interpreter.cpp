#include "arguments.hpp"
#include "interpreter.hpp"
#include "util.hpp"
#include "cl/opencl.h"
#include <cstring>
#include <cmath>
#include <atomic>
#include <cerrno>

namespace Loom {

static Unit __read_arg(ubyte const*&, ubyte const*, ubyte*);

static void __noop();
static void __debug(ubyte const*&, ubyte const*, ThreadPool::Stack*&);

static void __assg(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __push(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __pop(ThreadPool::Stack*&);
static void __make(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __free(ubyte const*&, ubyte const*, ThreadPool::Stack*&);

static void __call(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __sysc(ubyte const*&, ubyte const*, ThreadPool::Stack*&, FunctionTable::Local);
static bool __retn(ubyte const*&, ubyte const*, ThreadPool::Stack*&, ThreadPool*);

static void __jump(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __jz(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __jnz(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __jeq(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __jneq(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __jls(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __jlse(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __jgr(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __jgre(ubyte const*&, ubyte const*, ThreadPool::Stack*&);

static void __incr(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __decr(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __iadd(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __isub(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __imul(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __idiv(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __ipow(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __imod(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __isqrt(ubyte const*&, ubyte const*, ThreadPool::Stack*&);

static void __start(ubyte const*&, ubyte const*, ThreadPool::Stack*&, ThreadPool*);
static bool __stop(ubyte const*&, ubyte const*, ThreadPool::Stack*&, ThreadPool*);
static void __sync(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __copy(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __cnfg1(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __cnfg2(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __cnfg3(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __buffer(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __index1(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __index2(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __index3(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __karg(ubyte const*&, ubyte const*, ThreadPool::Stack*&);

static void __rpcs(ubyte const*&, ubyte const*, ThreadPool::Stack*&, ThreadPool*, FunctionTable::Local);
static void __kern1(ubyte const*&, ubyte const*, ThreadPool::Stack*&, ThreadPool*, FunctionTable::Local);

static void __file_open(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __file_close(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __file_read(ubyte const*&, ubyte const*, ThreadPool::Stack*&);
static void __file_write(ubyte const*&, ubyte const*, ThreadPool::Stack*&);

#ifdef _DEBUG_ADVANCED_
static void __DEBUG_print_frame(ubyte*, uint32);
#endif

void Interpreter::runProgram(ThreadPool& pool, FunctionTable& table, Code* program, int argc, char** argv) {
    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [+ Thread pool received program ]\n");
    #endif

    Loom::FunctionTable::Local local =
        table.localizeTableAgainstProgram(program);

    CODE::HEADER& hd = *((CODE::HEADER*) program);

    Code* inst = program + hd.ENTRY_POINT;

    struct arguments {
        Unit argc;
        char** argv;
    };
    arguments* args = new arguments({(Unit)argc, argv});
    ThreadPool::Request req = {
        ThreadPool::Stack::Return::ASYNC,
        0,
        inst,
        program,
        sizeof(arguments),
        (ubyte*)args,
        local
    };

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [+ Launching program ]\n");
    #endif
    pool.exec(req);
}

Interpreter::Interpreter(ThreadPool& pool) :
    _pool(pool) { ; }
Interpreter::~Interpreter() { ; }

void Interpreter::exec(ubyte const*& instr,
                       const ubyte* base,
                       ThreadPool::Stack*& stack,
                       ThreadPool* pool,
                       FunctionTable::Local ftab) {
    #ifdef _DEBUG_ADVANCED_
    ubyte const* instr_old = instr;
    #endif//_DEBUG_

    while (pool -> active()) {
        #ifdef _DISPLAY_ENTRY_
        fprintf(stderr, 
            "    [@ %lu (%x -- %s) (stack = %p) ]\n", 
            (std::size_t)(instr - base),
            *instr,
            CODE::toString(*instr),
            (void*)stack -> ptr());
        #endif//_DISPLAY_ENTRY_

        #ifdef _DEBUG_ADVANCED_
        try {
        #endif//_DEBUG_

            switch (*(instr++)) {
                case CODE::NOOP:
                    __noop();
                    break;
                case CODE::DEBUG:
                    __debug(instr, base, stack);
                    break;
                case CODE::ASSG:
                    __assg(instr, base, stack);
                    break;
                case CODE::PUSH:
                    __push(instr, base, stack);
                    break;
                case CODE::POP:
                    __pop(stack);
                    break;
                case CODE::MAKE:
                    __make(instr, base, stack);
                    break;
                case CODE::FREE:
                    __free(instr, base, stack);
                    break;
                case CODE::CALL:
                    __call(instr, base, stack);
                    break;
                case CODE::SYSC:
                    __sysc(instr, base, stack, ftab);
                    break;
                case CODE::RETN:
                    if ( !__retn(instr, base, stack, pool) )
                        goto __interpret_done;
                    break;
                case CODE::JUMP:
                    __jump(instr, base, stack);
                    break;
                case CODE::JZ:
                    __jz(instr, base, stack);
                    break;
                case CODE::JNZ:
                    __jnz(instr, base, stack);
                    break;
                case CODE::JEQ:
                    __jeq(instr, base, stack);
                    break;
                case CODE::JNEQ:
                    __jneq(instr, base, stack);
                    break;
                case CODE::JLS:
                    __jls(instr, base, stack);
                    break;
                case CODE::JLSE:
                    __jlse(instr, base, stack);
                    break;
                case CODE::JGR:
                    __jgr(instr, base, stack);
                    break;
                case CODE::JGRE:
                    __jgre(instr, base, stack);
                    break;
                case CODE::INCR:
                    __incr(instr, base, stack);
                    break;
                case CODE::DECR:
                    __decr(instr, base, stack);
                    break;
                case CODE::IADD:
                    __iadd(instr, base, stack);
                    break;
                case CODE::ISUB:
                    __isub(instr, base, stack);
                    break;
                case CODE::IMUL:
                    __imul(instr, base, stack);
                    break;
                case CODE::IDIV:
                    __idiv(instr, base, stack);
                    break;
                case CODE::IPOW:
                    __ipow(instr, base, stack);
                    break;
                case CODE::IMOD:
                    __imod(instr, base, stack);
                    break;
                case CODE::ISQRT:
                    __isqrt(instr, base, stack);
                    break;

                case CODE::START:
                    __start(instr, base, stack, pool);
                    break;
                case CODE::STOP:
                    if ( !__stop(instr, base, stack, pool) )
                        goto __interpret_done;
                    break;
                case CODE::SYNC:
                    __sync(instr, base, stack);
                    break;
                case CODE::COPY:
                    __copy(instr, base, stack);
                    break;
                case CODE::CNFG1:
                    __cnfg1(instr, base, stack);
                    break;
                case CODE::CNFG2:
                    __cnfg2(instr, base, stack);
                    break;
                case CODE::CNFG3:
                    __cnfg3(instr, base, stack);
                    break;
                case CODE::BUFFER:
                    __buffer(instr, base, stack);
                    break;
                case CODE::INDEX1:
                    __index1(instr, base, stack);
                    break;
                case CODE::INDEX2:
                    __index2(instr, base, stack);
                    break;
                case CODE::INDEX3:
                    __index3(instr, base, stack);
                    break;
                case CODE::KARG:
                    __karg(instr, base, stack);
                    break;
                    

                case CODE::RPCS:
                    __rpcs(instr, base, stack, pool, ftab);
                    break;
                case CODE::KERN1:
                    __kern1(instr, base, stack, pool, ftab);
                    break;

                case CODE::FILEOPEN:
                    __file_open(instr, base, stack);
                    break;
                case CODE::FILECLOSE:
                    __file_close(instr, base, stack);
                    break;
                case CODE::FILEREAD:
                    __file_read(instr, base, stack);
                    break;
                case CODE::FILEWRITE:
                    __file_write(instr, base, stack);
                    break;
                default:
                    sendError("Bad operator in assembly : %x (%d)", *(instr-1), (std::size_t)(instr - base));
                    break;
            }

        #ifdef _DEBUG_ADVANCED_
        } catch ( std::string e ) {
            fprintf(stderr, "-----+ Encountered an error here: -----------\n");
            fprintf(stderr, "%4d | ", (uint32)(instr_old-base));
            for ( ubyte const* i=instr_old;i<instr;i++ ) {
                fprintf(stderr, "%02x ", *i);
            }
            fprintf(stderr, "\n");
            fprintf(stderr, "-----+---------------------------------------\n");
            throw e;
        }
        instr_old = instr;
        #endif//_DEBUG_
    }
__interpret_done:
    return;
}

//===============================================================================
//
//===============================================================================

// <__read_arg> is the bottleneck of the interpreter.
// Any performance improvement here will drastically change the
// execution speed of any program run with the interpreter.

Unit __read_arg(ubyte const*& instr, ubyte const* root, ubyte* stack) {
    ubyte code = *instr;
    instr += sizeof(Code);

    Unit result = * (Unit*) instr;
    instr += sizeof(Unit);

    #ifdef _DISPLAY_ARGS_
    fprintf(stderr, "    [:: read arg %x -- %lu", code, result);
    #endif

    switch ( code ) {
        case CODE::VALUE:
            break;
        case CODE::DATA:
            result = (Unit)(root + result);
            break;
        case CODE::ADDR:
            result = (Unit) &stack[result];
            break;
        case CODE::LOCAL:
            result = * (Unit*) &stack[result];
            break;
        case CODE::GLOB:
            result = ** (Unit**) &stack[result];
            break;
        default:
            sendError("Invalid argument code %d detected", code);
    }

    #ifdef _DISPLAY_ARGS_
    fprintf(stderr, " = %lu]\n", result);
    #endif

    return result;
}

void __noop() {
    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! noop ]\n");
    #endif//_DEBUG_
}

void __debug(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit value = __read_arg(instr, root, frame);

    fprintf(stderr, "    [! DEBUG = '%s' ]\n", (const char*) value);
}

void __assg(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit value = __read_arg(instr, root, frame);
    Unit addr  = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! address[%p] = %lu ]\n", (void*) addr, value);
    #endif//_DEBUG_

    * (Unit*) addr = value;
}
void __push(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit value = __read_arg(instr, root, frame);
    
    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! push stack up %lu ]\n", value);
    #endif//_DEBUG_

    stack -> push(value);
}
void __pop(ThreadPool::Stack*& stack) {
    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! pop stack back ]\n");
    #endif//_DEBUG_

    stack -> pop();
}
void __make(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit size = __read_arg(instr, root, frame);
    Unit target = __read_arg(instr, root, frame);

    ubyte* buffer = (ubyte*) operator new[](size);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! make %lu byte buffer (%p) at address[%lu] ]\n", size, buffer, target - (Unit) frame);
    #endif//_DEBUG_

    * (ubyte**) target = buffer;
}
void __free(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit target = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! free memory at %p ]\n", (void*) target);
    #endif//_DEBUG_

    operator delete[]((ubyte*) target);
}
void __call(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit function = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! call function at %lu ]\n", function);
    #endif//_DEBUG_

    stack -> pushAddress(ThreadPool::Stack::Return::SERIAL, (Unit)(instr - root));
    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "Push %p(%lu) on %lu\n", stack, stack -> addressStackSize(), getThreadId());
    #endif//_DEBUG_
    instr = root + function;
}
void __sysc(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack, FunctionTable::Local ftab) {
    ubyte* frame = stack -> ptr();
    
    Unit function = * (Unit*) instr;
    instr += sizeof(Unit);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! run syscall[%lu] ]\n", function);
    #endif//_DEBUG_

    if ( !ftab )
        sendError("No function call table provided");

    ftab[function](frame);
}
bool __retn(ubyte const*& instr, ubyte const* base, ThreadPool::Stack*& stack, ThreadPool* pool) {
    return pool -> retn(instr, base, stack);
}
void __jump(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit function = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! jump from {%lu} to {%lu} ]\n", instr - root, function);
    #endif//_DEBUG_
    
    instr = root + function;
}
void __jz(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit function = __read_arg(instr, root, frame);
    Unit a = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! check (%lu == 0) {old=%lu, new=%lu} ]\n", a, instr - root, function);
    #endif//_DEBUG_
    if ( a == 0 ) {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! jump to %lu ]\n", function);
        #endif//_DEBUG_
        instr = root + function;
    } else {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! stay at %lu ]\n", instr - root);
        #endif//_DEBUG_
    }
}
void __jnz(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit function = __read_arg(instr, root, frame);
    Unit a = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! check (%lu != 0) {old=%lu, new=%lu} ]\n", a, instr - root, function);
    #endif//_DEBUG_
    if ( a != 0 ) {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! jump to %lu ]\n", function);
        #endif//_DEBUG_
        instr = root + function;
    } else {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! stay at %lu ]\n", instr - root);
        #endif//_DEBUG_
    }
}
void __jeq(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit function = __read_arg(instr, root, frame);

    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! check (%lu == %lu) {old=%lu, new=%lu} ]\n", a, b, instr - root, function);
    #endif//_DEBUG_
    if ( a == b ) {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! jump to %lu ]\n", function);
        #endif//_DEBUG_
        instr = root + function;
    } else {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! stay at %lu ]\n", instr - root);
        #endif//_DEBUG_
    }
}
void __jneq(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit function = __read_arg(instr, root, frame);

    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! check (%lu != %lu) {old=%lu, new=%lu} ]\n", a, b, instr - root, function);
    #endif//_DEBUG_
    if ( a != b ) {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! jump to %lu ]\n", function);
        #endif//_DEBUG_
        instr = root + function;
    } else {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! stay at %lu ]\n", instr - root);
        #endif//_DEBUG_
    }
}
void __jls(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit function = __read_arg(instr, root, frame);

    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! check (%lu < %lu) {old=%lu, new=%lu} ]\n", a, b, instr - root, function);
    #endif//_DEBUG_
    if ( a < b ) {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! jump to %lu ]\n", function);
        #endif//_DEBUG_
        instr = root + function;
    } else {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! stay at %lu ]\n", instr - root);
        #endif//_DEBUG_
    }
}
void __jlse(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit function = __read_arg(instr, root, frame);

    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! check (%lu <= %lu) {old=%lu, new=%lu} ]\n", a, b, instr - root, function);
    #endif//_DEBUG_
    if ( a <= b ) {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! jump to %lu ]\n", function);
        #endif//_DEBUG_
        instr = root + function;
    } else {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! stay at %lu ]\n", instr - root);
        #endif//_DEBUG_
    }
}
void __jgr(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit function = __read_arg(instr, root, frame);

    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! check (%lu > %lu) {old=%lu, new=%lu} ]\n", a, b, instr - root, function);
    #endif//_DEBUG_
    if ( a > b ) {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! jump to %lu ]\n", function);
        #endif//_DEBUG_
        instr = root + function;
    } else {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! stay at %lu ]\n", instr - root);
        #endif//_DEBUG_
    }
}
void __jgre(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();

    Unit function = __read_arg(instr, root, frame);

    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! check (%lu >= %lu) {old=%lu, new=%lu} ]\n", a, b, instr - root, function);
    #endif//_DEBUG_
    if ( a >= b ) {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! jump to %lu ]\n", function);
        #endif//_DEBUG_
        instr = root + function;
    } else {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! stay at %lu ]\n", instr - root);
        #endif//_DEBUG_
    }
}

void __incr(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit a = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! increment %p=%lu to %lu ]\n", (Unit*)a, *(Unit*)a, (*(Unit*)a)+1);
    #endif//_DEBUG_

    ++ (*(Unit*)a);
}
void __decr(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit a = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! increment %p=%lu to %lu ]\n", (Unit*)a, *(Unit*)a, (*(Unit*)a)+1);
    #endif//_DEBUG_

    -- (*(Unit*)a);
}
void __iadd(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);
    Unit c = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! %p = %lu + %lu ]\n", (Unit*) c, a, b);
    #endif//_DEBUG_

    * (Unit*) c = a + b;
}
void __isub(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);
    Unit c = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! %p = %lu - %lu ]\n", (Unit*) c, a, b);
    #endif//_DEBUG_

    * (Unit*) c = a - b;
}
void __imul(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);
    Unit c = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! %p = %lu * %lu ]\n", (Unit*) c, a, b);
    #endif//_DEBUG_

    * (Unit*) c = a * b;
}
void __idiv(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);
    Unit c = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! %p = %lu / %lu ]\n", (Unit*) c, a, b);
    #endif//_DEBUG_

    if ( b == 0 )
        sendError("Zero division error!");

    * (Unit*) c = a / b;
}
void __ipow(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);
    Unit c = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! %p = %lu ** %lu ]\n", (Unit*) c, a, b);
    #endif//_DEBUG_

    * (Unit*) c = std::pow(a, b);
}
void __imod(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);
    Unit c = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! %p = %lu %% %lu ]\n", (Unit*) c, a, b);
    #endif//_DEBUG_

    if ( b == 0 )
        sendError("Zero modulo error!");

    * (Unit*) c = a % b;
}
void __isqrt(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit a = __read_arg(instr, root, frame);
    Unit b = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! %p = sqrt(%lu) ]\n", (Unit*) b, a);
    #endif//_DEBUG_

    * (Unit*) b = std::sqrt(a);
}

void __start(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack, ThreadPool* pool) {
    ubyte* frame = stack -> ptr();
    Unit sync = pool -> syncAlloc();
    Unit loc = __read_arg(instr, root, frame);
    pool -> getSync(sync).reset();

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! load sync(%lu) to %p ]\n", sync, (void*)loc);
    #endif//_DEBUG_

    (* (Unit*) loc) = sync;
}
bool __stop(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack, ThreadPool* pool) {
    // STOP CASES
    // 1.) Have to wait for remotes to finish
    // 2.) Remotes have finished so continue
    // 3.) There were no remotes

    ubyte* frame = stack -> ptr();
    Unit sync_loc = __read_arg(instr, root, frame);
    CODE::Sync& sync = pool -> getSync(sync_loc);

    std::lock_guard<std::mutex> lk(sync.getLock());

    sync.setTarget((Unit)instr);
    sync.setStack((Unit)stack);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! wait on sync(%lu) ]\n", sync_loc);
    #endif//_DEBUG_

    if ( sync.getDep() == 0 ) {
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! no dependencies left ]\n");
        #endif//_DEBUG_
        pool -> syncDealloc(sync_loc);
        return true;
    }
    return false;
}

void __sync(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    stack -> sync = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! set sync value to %lu ]\n", stack -> sync);
    #endif//_DEBUG_
}
void __copy(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    stack -> copy = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! set copy value to %lu ]\n", stack -> copy);
    #endif//_DEBUG_
}

void __cnfg1(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    stack -> config[0] = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! set config[0] value to %lu ]\n", stack -> config[0]);
    #endif//_DEBUG_
}
void __cnfg2(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    stack -> config[1] = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! set config[1] value to %lu ]\n", stack -> config[1]);
    #endif//_DEBUG_
}
void __cnfg3(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    stack -> config[2] = __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! set config[2] value to %lu ]\n", stack -> config[2]);
    #endif//_DEBUG_
}
void __buffer(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    ubyte access_type = *instr;
    instr ++ ;

    Unit size = __read_arg(instr, root, frame);
    ubyte* buffer = (ubyte*) __read_arg(instr, root, frame);
    ubyte* target = (ubyte*) __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! buffer %p[%lu] to location %p ]\n", buffer, size, target);
    #endif//_DEBUG_

    stack -> pushBuffer(access_type, size, buffer, target);
    *(ubyte**) target = buffer;
}
void __index1(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit* location = (Unit*) __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! get index[0] = %lu ]\n", stack -> index[0]);
    #endif//_DEBUG_

    *location = stack -> index[0];
}
void __index2(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit* location = (Unit*) __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! get index[1] = %lu ]\n", stack -> index[1]);
    #endif//_DEBUG_

    *location = stack -> index[1];
}
void __index3(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit* location = (Unit*) __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! get index[2] = %lu ]\n", stack -> index[2]);
    #endif//_DEBUG_

    *location = stack -> index[2];
}
void __karg(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit offset = __read_arg(instr, root, frame);
    Unit* write = (Unit*) __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! write from kernel buffer at location %lu to %p ]\n", offset, write);
    #endif//_DEBUG_

    *write = * (Unit*) (stack -> base() + offset);
}

void __rpcs(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack, ThreadPool* pool, FunctionTable::Local ftab) {
    ubyte* frame = stack -> ptr();
    Unit function = __read_arg(instr, root, frame);

    if ( pool -> backlogTooBig() ) {

        stack -> pushAddress(ThreadPool::Stack::Return::SERIAL, (Unit)(instr - root));
        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "Push %p(%lu) on %lu\n", stack, stack -> addressStackSize(), getThreadId());
        #endif
        instr = root + function;

    } else {

        Unit copy_count = stack -> copy - stack -> addressStackTop();
        ubyte* data = new ubyte[copy_count];
        std::copy(frame, frame + copy_count, data);

         #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! copy %lu bytes of data to %p ]\n", copy_count, data);
        //__DEBUG_print_frame(data, stack -> copy);
        #endif//_DEBUG_

        {
            CODE::Sync& sync = pool -> getSync(stack -> sync);
            std::lock_guard<std::mutex> lk(sync.getLock());
            sync.addDep();
        }

         #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! launch remote call at %lu ]\n", function);
        #endif//_DEBUG_

        ThreadPool::Request req = {
            ThreadPool::Stack::Return::SYNC,
            stack -> sync,
            root + function,
            root,
            stack -> copy,
            data,
            ftab
        };
        pool -> exec(req);

    }
}

inline void __prepare_kern1_req(ThreadPool::Request& req, Unit func, Unit root, Unit start, Unit end, ThreadPool::Stack* stack, FunctionTable::Local ftab) {
    ubyte* frame = stack -> ptr();
    Unit copy_count = stack -> copy - stack -> addressStackTop();
    Unit data_size = copy_count + sizeof(Unit[3]);
    ubyte* data = new ubyte[data_size];

    ((Unit*)data)[0] = 1;
    ((Unit*)data)[1] = start;
    ((Unit*)data)[2] = end;

    std::copy(frame, frame + copy_count, data + sizeof(Unit[3]));

    req.retn_type = ThreadPool::Stack::Return::KERN;
    req.retn_sync = stack -> sync;
    req.instruction = (ubyte const*) root + func;
    req.root = (ubyte const*) root;
    req.data_size = data_size;
    req.data = data;
    req.ftable = ftab;
}

void __kern1(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack, ThreadPool* pool, FunctionTable::Local ftab) {
    ubyte* frame = stack -> ptr();
    Unit function = __read_arg(instr, root, frame);

    bool exec_on_device = false;

    if ( exec_on_device ) {

        // not yet implemented

    } else {

        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! launch kernel on host ]\n");
        #endif//_DEBUG_

        Unit count = stack -> config[0];
        Unit reqs = pool -> threadCount();
        Unit count_per_req = count / reqs;

        Unit iter = 0;
        ThreadPool::Request req;

        {
            CODE::Sync& sync = pool -> getSync(stack -> sync);
            std::lock_guard<std::mutex> lk(sync.getLock());
            for ( Unit i=0;i<reqs;i++ )
                sync.addDep();
        }

        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! kernel with %lu operations broken up into %lu blocks ]\n", count, reqs);
        #endif//_DEBUG_

        for ( Unit i=0;i<reqs-1;i++ ) {
            __prepare_kern1_req(req, function, (Unit)root, iter, iter + count_per_req, stack, ftab);

            #ifdef _DEBUG_ADVANCED_
            fprintf(stderr, "    [! launch request %lu through %lu ]\n", iter, iter + count_per_req);
            #endif//_DEBUG_

            pool -> exec(req);
            iter += count_per_req;
        }

        __prepare_kern1_req(req, function, (Unit)root, iter, count, stack, ftab);

        #ifdef _DEBUG_ADVANCED_
        fprintf(stderr, "    [! launch request %lu through %lu ]\n", iter, count);
        #endif//_DEBUG_

        pool -> exec(req);
    }
}

void __file_open(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    const char* fname = (const char*) __read_arg(instr, root, frame);
    const char* opt = (const char*) __read_arg(instr, root, frame);
    FILE** file = (FILE**) __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! open file '%s' with options '%s' to address (", fname, opt);
    #endif//_DEBUG_

    *file = std::fopen(fname, opt);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "%p) (%d) ]\n", *file, errno);
    #endif//_DEBUG_
}
void __file_close(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    FILE* file = (FILE*) __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! close file %p ]\n", file);
    #endif//_DEBUG_

    std::fclose(file);
}
void __file_read(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit buffer_size = __read_arg(instr, root, frame);
    FILE* file = (FILE*) __read_arg(instr, root, frame);
    ubyte* buffer = (ubyte*) __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! read %lu bytes from file %p to buffer %p ]\n", buffer_size, file, buffer);
    #endif//_DEBUG_

    if ( !file ) {
        sendError("Cannot read from invalid file");
    }
    std::fread(buffer, 1, buffer_size, file);
}
void __file_write(ubyte const*& instr, ubyte const* root, ThreadPool::Stack*& stack) {
    ubyte* frame = stack -> ptr();
    Unit buffer_size = __read_arg(instr, root, frame);
    ubyte* buffer = (ubyte*) __read_arg(instr, root, frame);
    FILE* file = (FILE*) __read_arg(instr, root, frame);

    #ifdef _DEBUG_ADVANCED_
    fprintf(stderr, "    [! write %lu bytes from buffer %p to file %p ]\n", buffer_size, buffer, file);
    #endif//_DEBUG_

    if ( !file ) {
        sendError("Cannot write to invalid file");
    }
    std::fwrite(buffer, 1, buffer_size, file);
}

//===============================================================================
//
//===============================================================================

#ifdef _DEBUG_ADVANCED_
static void __DEBUG_print_frame(ubyte* addr, uint32 count) {
    fprintf(stderr, "+-- DEBUG_print_frame ----------------------------+\n| %p\n", addr);
    for ( uint32 i=0;i<count; ) {
        fprintf(stderr, "| ");
        for ( uint32 j=0;j<16&&i<count;j++,i++ ) {
            fprintf(stderr, "%02x ", addr[i]);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "+-------------------------------------------------+\n");
}
#endif//_DEBUG_

}
