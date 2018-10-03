#ifndef __LOOM_ASSEMBLER_HPP__
#define __LOOM_ASSEMBLER_HPP__

#include "types.hpp"
#include "code.hpp"
#include "lexer_stream.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

namespace Loom {

    struct Object {

        Object(const std::string& raw_yarn_data);

        // assemble and add in symbols from another YARN file
        void include(const std::string& yarn_file_name);

        // reset the internal state of the object
        void reset(); 

        // assemble the object skeleton program
        void assemble(); 

        // link the program with references
        void link(); 

        // return a const reference to <_program>
        // note this vector is empty until a call
        // to assembler and link
        const std::vector<uint8_t>& getProgram() const;

        static const uint32 PARSE_INT,
                            PARSE_FLOAT,
                            PARSE_BOTH;

        // Type bytecodes prior to the link phase
        static const Code ASM_TYPE_VALUE,
                          ASM_TYPE_TEXT,
                          ASM_TYPE_DATA,
                          ASM_TYPE_LOCAL,
                          ASM_TYPE_ADDR,
                          ASM_TYPE_GLOB,

                          ASM_TYPE_LITERAL,
                          ASM_TYPE_VARIABLE,
                          ASM_TYPE_ANY;

    private:

        struct __conf_lexer__ {
            __conf_lexer__(Lexer&);
        };
        struct __kernel_block__ {
            Unit text_start; // where in <_text> the kernel starts
            Unit text_end; // where in <_text> the kernel ends (non-inclusive)
            Unit call_start;
            Unit call_end;
        };
        struct __kernel_call__ {
            std::string reference_name; // name of the call
            uint32 line; // line in the source file the call occurred at
            uint32 column; // column in the source file the call occurred at
        };

        static void __text_write_argument(std::vector<uint8_t>&, Code, Unit, Unit);
        static void __text_read_argument(std::vector<uint8_t>&, Code&, Unit&, Unit);

        typedef std::unordered_map<Unit, std::string> __needs_reference_t__;
        typedef std::unordered_map<std::string, Unit> __references_t__;
        typedef std::unordered_map<Unit, Code> __allowed_types_t__;
        typedef std::unordered_map<std::string, Code> __types_t__;

        // Input token object
        Lexer _lexer;

        // Configure the lexer with correct character
        // sequences at construction time
        __conf_lexer__ _lexer_config;

        // Lexer stream to fetch data
        LexerStream _stream;

        // Keeps track of program text and variable names
        std::vector<uint8_t> _text;
        __needs_reference_t__ _needs_reference; // a unit of memory that needs to be written at link time
        __references_t__ _references; // a named reference and its unit value
        __types_t__ _types; // sets the type of a named reference
        __allowed_types_t__ _allowed;

        // Keeps track of string data for the program
        std::vector<uint8_t> _data;
        __references_t__ _string_map;
        std::string _anon_varname; // Name for anonymous variables type to strings
                                   // The name begins with a and is incremented every
                                   // time a new name is needed. The name is prepended
                                   // with the illegal character '+' to avoid name
                                   // overlap.

        // Keeps track of system function calls
        std::vector<uint8_t> _func; // Sequentially stored, null-character separated strings
        uint32 _func_count; // number of functions stored
        __references_t__ _system_calls; // keeps track of what system calls
                                        // have been defined, and what indices
                                        // in the <_func > buffer their names
                                        // are located at.

        // Keeps track of where kernel calls are and what jumps are allowed
        std::vector<__kernel_call__> _kernel_calls;
        std::vector<__kernel_block__> _kernel;
        bool _in_kernel_mode; // determines if a jump call should be scrutinized at
                              // link time to see if it leaves its associated kernel

        // Finalized program that is built during link time
        std::vector<uint8_t> _program;

        // Meta data for @stack attributes
        std::stack<Unit> _meta_stack_pointers;

        // Meta data for @search statments
        std::vector<std::string> _meta_search;

        // Meta data for @entry
        Unit _meta_entry_point;

        // Meta data for @parse_args
        bool _meta_parse_args;

        // Helpers
        Unit __prefix_value();

        void __arg_assign(Code, Unit, Unit);
        void __arg_reference(Code, Code, const std::string&, Unit);
        void __arg_prefix(Unit);
        void __arg_symbol(Unit);
        void __arg_token(Unit);
        void __arg_literal(Unit);
        void __arg_string(Unit);
        void __arg_paran(Unit);
        void __arg_scope(Unit);
        void __arg_bracket(Unit);
        void __argument(const char*, uint32, Code);

        void __text_code(Code);
        void __text_unit(Unit);
        void __text_argument(Code, Unit);

        void __set_reference(const std::string&, Unit);
        void __set_type(const std::string&, Code);

        const std::string& __get_anon_varname();
        bool __token_is_variable();
        Unit __get_token_value();

        // Meta Compilation
        void __meta_push_function();
        void __meta_pop_function();

        void __meta_stack_unit(Unit);
        void __meta_stack_addr(Unit);
        void __meta_stack_sync(Unit);

        void __meta_search();
        void __meta_use();
        void __meta_parse_args();
        void __meta_entry();
        void __meta_function();
        void __meta_end_function();
        void __meta_kernel();
        void __meta_end_kernel();

        // Statment Processing
        void __stmt_opcode();
        void __stmt_metacode();
        void __stmt_reference();
        void __stmt_assign();

        // Program Debugging
        void __noop();
        void __debug();

        // Memory Movement and Flow Control
        void __assg(); // ASSG value ; output-ptr
        void __sysc(); // SYSC syscl-string
        void __call(); // CALL func-ptr
        void __retn(); // RETN
        void __push(); // PUSH add-to-stack
        void __pop();  // POP
        void __make(); // MAKE buffer-size ; output-ptr
        void __free(); // FREE output-ptr

        void __jump(); // JUMP func-ptr
        void __jeq();  // JEQ func-ptr lhs rhs
        void __jneq(); // JNEQ func-ptr lhs rhs
        void __jls();  // JLS func-ptr lhs rhs
        void __jlse(); // JLSE func-ptr lhs rhs
        void __jgr();  // JGR func-ptr lhs rhs
        void __jgre(); // JGRE func-ptr lhs rhs
        void __jz();   // JZ func-ptr value
        void __jnz();  // JNZ func-ptr value

        // Integer math
        void __incr(); // INCR value-ptr
                       // INCR value ; output-ptr
        void __decr(); // DECR value-ptr
                       // DECR value ; output-ptr
        void __ineg();
        void __iadd();
        void __isub();
        void __imul();
        void __idiv();
        void __ipow();
        void __imod();
        void __isqrt();
        void __ilog();

        // Floating point math
        void __sin();
        void __cos();
        void __tan();
        void __fneg();
        void __fadd();
        void __fsub();
        void __fmul();
        void __fdiv();
        void __fpow();
        void __fmod();
        void __fsqrt();
        void __flog();

        // Floating point vector math
        void __fvneg();
        void __fvadd();
        void __fvsub();
        void __fvmul();
        void __fvdiv();
        void __fvdot();
        void __fvcross();

        // Matrix math
        void __mneg();
        void __madd();
        void __msub();
        void __mmul();
        void __mtrans();

        // Parallel Execution
        void __rpc();
        void __rpcs();
        void __kernel1();
        void __kernel2();
        void __kernel3();
        void __reduction();
        void __scan();
        void __sort();
        void __stable_sort();

        // Parallel Setup
        void __start();
        void __stop();
        void __sync();
        void __config1();
        void __config2();
        void __config3();
        void __copy();
        void __buffer();
        void __index1();
        void __index2();
        void __index3();
        void __karg();

        // File Access Functions
        void __file_open();
        void __file_close();
        void __file_read();
        void __file_write();
        void __file_seek();
        void __file_tell();
        void __file_reset();

        // Generic Functions
        void __printf();
        void __fprintf();
        void __memcpy();

    };

}

#endif//__LOOM_ASSEMBLER_HPP__