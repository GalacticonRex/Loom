#include "code.hpp"

#define LOOM_translate_code_to_name(NAME) \
    case CODE::NAME: return #NAME; break;

#ifdef _CYGWIN_BUILD_
#define LOOM_define_code_value(NAME, VALUE) \
    const Code CODE::NAME = VALUE;
#else
#define LOOM_define_code_value(NAME, VALUE)
#endif

namespace Loom {

    /*struct Bytecode {
            ubyte code;
            ArgType arg_type[3];
            Unit arg_value[3];
        };

    void Encode(ubyte&* output, const Bytecode& input) {
        *(output++) = input.code;
        switch(input.code) {
            // Dimension 4
            // Dimension 3
            // Dimension 2
            case ASSG:
            case MAKE:
            case JZ:
            case JNZ:
            case INEG:
            case ISQRT:
            case SIN:
            case COS:
            case TAN:
            case FNEG:
            case FSQRT:


            // Dimension 1
            case DEBUG:
            case PUSH:
            case FREE:
            case CALL:
            case SYSC:
            case JUMP:
            case INCR:
            case DECR:
            case RPC:
            case RPCS:
            case KERN1:
            case KERN2:
            case KERN3:
            case REDC:
            case SCAN:
            case SORT:
            case SSORT:
            case START:
            case STOP:
            case SYNC:
            case COPY:
            case CNFG1:
            case CNFG2:
            case CNFG3:
            case INDEX1:
            case INDEX2:
            case INDEX3:
                break;
            // Dimension 0
            case NOOP:
            case POP:
            case RETN:
                break;
        }
    }
    void Decode(Bytecode& output, ubyte&* input) {

    }*/

    //===================================
    // Constant Values
    //===================================

    LOOM_define_code_value(CONST_READ,      0xe0);
    LOOM_define_code_value(CONST_WRITE,     0xe1);
    LOOM_define_code_value(CONST_READWRITE, 0xe2);

    //===================================
    // Memory Handling
    //===================================

    LOOM_define_code_value(VALUE,  0xf0);
    LOOM_define_code_value(LOCAL,  0xf1);
    LOOM_define_code_value(ADDR,   0xf2);
    LOOM_define_code_value(DATA,   0xf4);
    LOOM_define_code_value(GLOB,   0xf5);

    //===================================
    // BYTE CODE VALUES
    //===================================

    LOOM_define_code_value(NOOP,0x00);
    LOOM_define_code_value(DEBUG,0xff);

    // Memory Control
    LOOM_define_code_value(ASSG,0x01);
    LOOM_define_code_value(PUSH,0x02);
    LOOM_define_code_value(POP, 0x03);
    LOOM_define_code_value(MAKE,0x04);
    LOOM_define_code_value(FREE,0x05);

    // Flow Control
    LOOM_define_code_value(CALL,0x10);
    LOOM_define_code_value(SYSC,0x12);
    LOOM_define_code_value(RETN,0x13);
    LOOM_define_code_value(JUMP,0x14);
    LOOM_define_code_value(JEQ, 0x15);
    LOOM_define_code_value(JNEQ,0x16);
    LOOM_define_code_value(JLS, 0x17);
    LOOM_define_code_value(JLSE,0x18);
    LOOM_define_code_value(JGR, 0x19);
    LOOM_define_code_value(JGRE,0x1a);
    LOOM_define_code_value(JZ,  0x1b);
    LOOM_define_code_value(JNZ, 0x1c);

    // Integer Manipulation
    LOOM_define_code_value(INCR, 0x20);
    LOOM_define_code_value(DECR, 0x21);
    LOOM_define_code_value(INEG, 0x22);
    LOOM_define_code_value(IADD, 0x23);
    LOOM_define_code_value(ISUB, 0x24);
    LOOM_define_code_value(IMUL, 0x25);
    LOOM_define_code_value(IDIV, 0x26);
    LOOM_define_code_value(IPOW, 0x27);
    LOOM_define_code_value(IMOD, 0x28);
    LOOM_define_code_value(ISQRT,0x29);
    LOOM_define_code_value(ILOG, 0x2a);

    // Floating point
    LOOM_define_code_value(SIN,  0x30);
    LOOM_define_code_value(COS,  0x31);
    LOOM_define_code_value(TAN,  0x32);
    LOOM_define_code_value(FNEG, 0x33);
    LOOM_define_code_value(FADD, 0x34);
    LOOM_define_code_value(FSUB, 0x35);
    LOOM_define_code_value(FMUL, 0x36);
    LOOM_define_code_value(FDIV, 0x37);
    LOOM_define_code_value(FPOW, 0x38);
    LOOM_define_code_value(FMOD, 0x39);
    LOOM_define_code_value(FSQRT,0x3a);
    LOOM_define_code_value(FLOG, 0x3b);

    // Floating point vector
    LOOM_define_code_value(FVNEG,  0x40);
    LOOM_define_code_value(FVADD,  0x41);
    LOOM_define_code_value(FVSUB,  0x42);
    LOOM_define_code_value(FVMUL,  0x43);
    LOOM_define_code_value(FVDIV,  0x44);
    LOOM_define_code_value(FVDOT,  0x45);
    LOOM_define_code_value(FVCROSS,0x46);

    // Matrix
    LOOM_define_code_value(MNEG,  0x50);
    LOOM_define_code_value(MADD,  0x51);
    LOOM_define_code_value(MSUB,  0x52);
    LOOM_define_code_value(MMUL,  0x53);
    LOOM_define_code_value(MTRANS,0x54);

    // Advanced Parallel Primitives
    LOOM_define_code_value(RPC,   0x60);
    LOOM_define_code_value(RPCS,  0x61);
    LOOM_define_code_value(KERN1, 0x62);
    LOOM_define_code_value(KERN2, 0x63);
    LOOM_define_code_value(KERN3, 0x64);
    LOOM_define_code_value(REDC,  0x65);
    LOOM_define_code_value(SCAN,  0x66);
    LOOM_define_code_value(SORT,  0x67);
    LOOM_define_code_value(SSORT, 0x68);

    // Parallel Setup Operations
    LOOM_define_code_value(START,  0x70);
    LOOM_define_code_value(STOP,   0x71);
    LOOM_define_code_value(SYNC,   0x72);
    LOOM_define_code_value(COPY,   0x73);
    LOOM_define_code_value(CNFG1,  0x74);
    LOOM_define_code_value(CNFG2,  0x75);
    LOOM_define_code_value(CNFG3,  0x76);
    LOOM_define_code_value(BUFFER, 0x77);
    LOOM_define_code_value(INDEX1, 0x78);
    LOOM_define_code_value(INDEX2, 0x79);
    LOOM_define_code_value(INDEX3, 0x7a);
    LOOM_define_code_value(KARG,   0x7b);

    // File Interaction
    LOOM_define_code_value(FILEOPEN,  0x80);
    LOOM_define_code_value(FILECLOSE, 0x81);
    LOOM_define_code_value(FILEREAD,  0x82);
    LOOM_define_code_value(FILEWRITE, 0x83);
    LOOM_define_code_value(FILESEEK,  0x84);
    LOOM_define_code_value(FILETELL,  0x85);
    LOOM_define_code_value(FILERESET, 0x86);

    // Basic Functions
    LOOM_define_code_value(PRINTF,  0xa0);
    LOOM_define_code_value(FPRINTF, 0xa1);
    LOOM_define_code_value(MEMCPY,  0xa2);

    const Unit CODE::HEADER::FLAG_PARSE_ARGS = 0x1;
    
    CODE::Sync::Sync() :
        _reliant(0),
        _load_target(0),
        _page(0) { ; }

    void CODE::Sync::reset() {
        std::lock_guard<std::mutex> lk(_lock);
        _reliant = 0;
        _load_target = 0;
        _page = 0;
    }

    std::mutex& CODE::Sync::getLock() {
        return _lock;
    }
    void CODE::Sync::setTarget(Unit targ) {
        _load_target = targ;
    }
    void CODE::Sync::setStack(Unit page) {
        _page = page;
    }
    Unit CODE::Sync::getTarget() const {
        return _load_target;
    }
    Unit CODE::Sync::getStack() const {
        return _page;
    }
    Unit CODE::Sync::getDep() const{
        return _reliant;
    }
    Unit CODE::Sync::addDep() {
        return ++ _reliant;
    }
    Unit CODE::Sync::removeDep() {
        return -- _reliant;
    }

    const char* CODE::toString(Code c) {
        switch(c) {
            LOOM_translate_code_to_name(CONST_READ);
            LOOM_translate_code_to_name(CONST_WRITE);
            LOOM_translate_code_to_name(CONST_READWRITE);

            LOOM_translate_code_to_name(VALUE);
            LOOM_translate_code_to_name(LOCAL);
            LOOM_translate_code_to_name(ADDR);
            LOOM_translate_code_to_name(DATA);
            LOOM_translate_code_to_name(GLOB);

            LOOM_translate_code_to_name(DEBUG);
            LOOM_translate_code_to_name(NOOP);
            LOOM_translate_code_to_name(ASSG);
            LOOM_translate_code_to_name(PUSH);
            LOOM_translate_code_to_name(POP);
            LOOM_translate_code_to_name(MAKE);
            LOOM_translate_code_to_name(FREE);
            LOOM_translate_code_to_name(CALL);
            LOOM_translate_code_to_name(SYSC);
            LOOM_translate_code_to_name(RETN);
            LOOM_translate_code_to_name(JUMP);
            LOOM_translate_code_to_name(JEQ);
            LOOM_translate_code_to_name(JNEQ);
            LOOM_translate_code_to_name(JLS);
            LOOM_translate_code_to_name(JLSE);
            LOOM_translate_code_to_name(JGR);
            LOOM_translate_code_to_name(JGRE);
            LOOM_translate_code_to_name(JZ);
            LOOM_translate_code_to_name(JNZ);
            LOOM_translate_code_to_name(INCR);
            LOOM_translate_code_to_name(DECR);
            LOOM_translate_code_to_name(INEG);
            LOOM_translate_code_to_name(IADD);
            LOOM_translate_code_to_name(ISUB);
            LOOM_translate_code_to_name(IMUL);
            LOOM_translate_code_to_name(IDIV);
            LOOM_translate_code_to_name(IPOW);
            LOOM_translate_code_to_name(IMOD);
            LOOM_translate_code_to_name(ISQRT);
            LOOM_translate_code_to_name(ILOG);
            LOOM_translate_code_to_name(SIN);
            LOOM_translate_code_to_name(COS);
            LOOM_translate_code_to_name(TAN);
            LOOM_translate_code_to_name(FNEG);
            LOOM_translate_code_to_name(FADD);
            LOOM_translate_code_to_name(FSUB);
            LOOM_translate_code_to_name(FMUL);
            LOOM_translate_code_to_name(FDIV);
            LOOM_translate_code_to_name(FPOW);
            LOOM_translate_code_to_name(FMOD);
            LOOM_translate_code_to_name(FSQRT);
            LOOM_translate_code_to_name(FLOG);
            LOOM_translate_code_to_name(FVNEG);
            LOOM_translate_code_to_name(FVADD);
            LOOM_translate_code_to_name(FVSUB);
            LOOM_translate_code_to_name(FVMUL);
            LOOM_translate_code_to_name(FVDIV);
            LOOM_translate_code_to_name(FVDOT);
            LOOM_translate_code_to_name(FVCROSS);
            LOOM_translate_code_to_name(MNEG);
            LOOM_translate_code_to_name(MADD);
            LOOM_translate_code_to_name(MSUB);
            LOOM_translate_code_to_name(MMUL);
            LOOM_translate_code_to_name(MTRANS);
            LOOM_translate_code_to_name(RPC);
            LOOM_translate_code_to_name(RPCS);
            LOOM_translate_code_to_name(KERN1);
            LOOM_translate_code_to_name(KERN2);
            LOOM_translate_code_to_name(KERN3);
            LOOM_translate_code_to_name(REDC);
            LOOM_translate_code_to_name(SCAN);
            LOOM_translate_code_to_name(SORT);
            LOOM_translate_code_to_name(SSORT);
            LOOM_translate_code_to_name(START);
            LOOM_translate_code_to_name(STOP);
            LOOM_translate_code_to_name(SYNC);
            LOOM_translate_code_to_name(CNFG1);
            LOOM_translate_code_to_name(CNFG2);
            LOOM_translate_code_to_name(CNFG3);
            LOOM_translate_code_to_name(COPY);
            LOOM_translate_code_to_name(BUFFER);
            LOOM_translate_code_to_name(INDEX1);
            LOOM_translate_code_to_name(INDEX2);
            LOOM_translate_code_to_name(INDEX3);
            LOOM_translate_code_to_name(KARG);
            LOOM_translate_code_to_name(FILEOPEN);
            LOOM_translate_code_to_name(FILECLOSE);
            LOOM_translate_code_to_name(FILEREAD);
            LOOM_translate_code_to_name(FILEWRITE);
            LOOM_translate_code_to_name(FILESEEK);
            LOOM_translate_code_to_name(FILETELL);
            LOOM_translate_code_to_name(FILERESET);
            LOOM_translate_code_to_name(PRINTF);
            LOOM_translate_code_to_name(FPRINTF);
            LOOM_translate_code_to_name(MEMCPY);

            default:
                return "UNKN";
                break;
        }
    }

}
