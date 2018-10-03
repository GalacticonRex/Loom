#ifndef __LOOM_CODES_HPP__
#define __LOOM_CODES_HPP__

#include <cwchar>
#include <cstdint>
#include <mutex>

#ifdef _CYGWIN_BUILD_
#define LOOM_declare_code_value(NAME, VALUE) \
    static const Code NAME;
#else
#define LOOM_declare_code_value(NAME, VALUE) \
    static const Code NAME = VALUE;
#endif

namespace Loom {

    typedef uint8_t  Code;
    typedef uint64_t Unit;

    typedef void (*ASM_FUNC)(Code*);

    struct CODE {

        //===================================
        // Constant Values
        //===================================

        LOOM_declare_code_value(CONST_READ,      0xe0);
        LOOM_declare_code_value(CONST_WRITE,     0xe1);
        LOOM_declare_code_value(CONST_READWRITE, 0xe2);

        //===================================
        // Memory Handling
        //===================================

        LOOM_declare_code_value(VALUE,  0xf0);
        LOOM_declare_code_value(LOCAL,  0xf1);
        LOOM_declare_code_value(ADDR,   0xf2);
        LOOM_declare_code_value(DATA,   0xf4);
        LOOM_declare_code_value(GLOB,   0xf5);

        //===================================
        // BYTE CODE VALUES
        //===================================

        LOOM_declare_code_value(NOOP,0x00);
        LOOM_declare_code_value(DEBUG,0xff);

        // Memory Control
        LOOM_declare_code_value(ASSG,0x01);
        LOOM_declare_code_value(PUSH,0x02);
        LOOM_declare_code_value(POP, 0x03);
        LOOM_declare_code_value(MAKE,0x04);
        LOOM_declare_code_value(FREE,0x05);

        // Flow Control
        LOOM_declare_code_value(CALL,0x10);
        LOOM_declare_code_value(SYSC,0x12);
        LOOM_declare_code_value(RETN,0x13);
        LOOM_declare_code_value(JUMP,0x14);
        LOOM_declare_code_value(JEQ, 0x15);
        LOOM_declare_code_value(JNEQ,0x16);
        LOOM_declare_code_value(JLS, 0x17);
        LOOM_declare_code_value(JLSE,0x18);
        LOOM_declare_code_value(JGR, 0x19);
        LOOM_declare_code_value(JGRE,0x1a);
        LOOM_declare_code_value(JZ,  0x1b);
        LOOM_declare_code_value(JNZ, 0x1c);

        // Integer Manipulation
        LOOM_declare_code_value(INCR, 0x20);
        LOOM_declare_code_value(DECR, 0x21);
        LOOM_declare_code_value(INEG, 0x22);
        LOOM_declare_code_value(IADD, 0x23);
        LOOM_declare_code_value(ISUB, 0x24);
        LOOM_declare_code_value(IMUL, 0x25);
        LOOM_declare_code_value(IDIV, 0x26);
        LOOM_declare_code_value(IPOW, 0x27);
        LOOM_declare_code_value(IMOD, 0x28);
        LOOM_declare_code_value(ISQRT,0x29);
        LOOM_declare_code_value(ILOG, 0x2a);

        // Floating point
        LOOM_declare_code_value(SIN,  0x30);
        LOOM_declare_code_value(COS,  0x31);
        LOOM_declare_code_value(TAN,  0x32);
        LOOM_declare_code_value(FNEG, 0x33);
        LOOM_declare_code_value(FADD, 0x34);
        LOOM_declare_code_value(FSUB, 0x35);
        LOOM_declare_code_value(FMUL, 0x36);
        LOOM_declare_code_value(FDIV, 0x37);
        LOOM_declare_code_value(FPOW, 0x38);
        LOOM_declare_code_value(FMOD, 0x39);
        LOOM_declare_code_value(FSQRT,0x3a);
        LOOM_declare_code_value(FLOG, 0x3b);

        // Floating point vector
        LOOM_declare_code_value(FVNEG,  0x40);
        LOOM_declare_code_value(FVADD,  0x41);
        LOOM_declare_code_value(FVSUB,  0x42);
        LOOM_declare_code_value(FVMUL,  0x43);
        LOOM_declare_code_value(FVDIV,  0x44);
        LOOM_declare_code_value(FVDOT,  0x45);
        LOOM_declare_code_value(FVCROSS,0x46);

        // Matrix
        LOOM_declare_code_value(MNEG,  0x50);
        LOOM_declare_code_value(MADD,  0x51);
        LOOM_declare_code_value(MSUB,  0x52);
        LOOM_declare_code_value(MMUL,  0x53);
        LOOM_declare_code_value(MTRANS,0x54);

        // Advanced Parallel Primitives
        LOOM_declare_code_value(RPC,   0x60);
        LOOM_declare_code_value(RPCS,  0x61);
        LOOM_declare_code_value(KERN1, 0x62);
        LOOM_declare_code_value(KERN2, 0x63);
        LOOM_declare_code_value(KERN3, 0x64);
        LOOM_declare_code_value(REDC,  0x65);
        LOOM_declare_code_value(SCAN,  0x66);
        LOOM_declare_code_value(SORT,  0x67);
        LOOM_declare_code_value(SSORT, 0x68);

        // Parallel Setup Operations
        LOOM_declare_code_value(START,  0x70);
        LOOM_declare_code_value(STOP,   0x71);
        LOOM_declare_code_value(SYNC,   0x72);
        LOOM_declare_code_value(COPY,   0x73);
        LOOM_declare_code_value(CNFG1,  0x74);
        LOOM_declare_code_value(CNFG2,  0x75);
        LOOM_declare_code_value(CNFG3,  0x76);
        LOOM_declare_code_value(BUFFER, 0x77);
        LOOM_declare_code_value(INDEX1, 0x78);
        LOOM_declare_code_value(INDEX2, 0x79);
        LOOM_declare_code_value(INDEX3, 0x7a);
        LOOM_declare_code_value(KARG,   0x7b);

        // File Interaction
        LOOM_declare_code_value(FILEOPEN,  0x80);
        LOOM_declare_code_value(FILECLOSE, 0x81);
        LOOM_declare_code_value(FILEREAD,  0x82);
        LOOM_declare_code_value(FILEWRITE, 0x83);
        LOOM_declare_code_value(FILESEEK,  0x84);
        LOOM_declare_code_value(FILETELL,  0x85);
        LOOM_declare_code_value(FILERESET, 0x86);

        // Basic Functions
        LOOM_declare_code_value(PRINTF,  0xa0);
        LOOM_declare_code_value(FPRINTF, 0xa1);
        LOOM_declare_code_value(MEMCPY,  0xa2);

        // header should always be 1024 bytes (128 units) in size
        struct HEADER {
            static const Unit FLAG_PARSE_ARGS;

            Unit VERSION[3];        //   3
            Unit FLAGS;             //   4
            Unit PROG_SIZE;         //   5
            Unit __buffer0[59];     //  64
            Unit CALL_TABLE_OFFSET; //  65
            Unit CALL_TABLE_SIZE;   //  66
            Unit __buffer1[6];      //  72
            Unit DATA_OFFSET;       //  73
            Unit DATA_SIZE;         //  74
            Unit __buffer2[6];      //  80
            Unit TEXT_OFFSET;       //  81
            Unit TEXT_SIZE;         //  82
            Unit __buffer3[6];      //  88
            Unit KERNEL_OFFSET;     //  89
            Unit KERNEL_SIZE;       //  90
            Unit __buffer4[37];     // 127
            Unit ENTRY_POINT;       // 128
        };

        struct Sync {
        private:
            
            Unit _reliant;
            Unit _load_target;
            Unit _page;
            std::mutex _lock;

        public:

            Sync();
            
            void reset();

            std::mutex& getLock();

            void setTarget(Unit);
            void setStack(Unit);
            Unit getTarget() const;
            Unit getStack() const;

            Unit getDep() const;
            Unit addDep();
            Unit removeDep();

        };

        static const char* toString(Code);

    };

}

#endif//__LOOM_CODES_HPP__
