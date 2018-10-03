#ifndef __LOOM_COMPILER_HPP__
#define __LOOM_COMPILER_HPP__

#include "lexer_stream.hpp"
#include <stack>

namespace Loom {

    struct Compiler {

        /* variable
         * type
         * array
         * function // container
         * tuple // container
         */

        typedef unsigned char Type;
        static const Type VAR_START;
        static const Type VAR_END;
        static const Type TYPE_START;
        static const Type TYPE_END;
        static const Type ARRAY_START;
        static const Type ARRAY_END;
        static const Type FUNC_INPUT;
        static const Type FUNC_OUTPUT;
        static const Type FUNC_END;
        static const Type TUPLE_START;
        static const Type TUPLE_END;

        enum class ScopeType {
            TEXTILE,
            FUNCTION
        };

        struct Instance {

        };

    private:

        std::stack<std::string> _namespace;
        std::stack<ScopeType> _namespace_type;
        std::unordered_map<std::string, std::vector<Type>> _type_map;
        std::unordered_map<std::string, Instance> _inst_map;

        bool __textile(LexerStream&);

        bool __expr(LexerStream&, std::vector<Type>&);
        bool __function_def(LexerStream&);
        bool __function_args(LexerStream&);

        bool __statement(LexerStream&);
        bool __import(LexerStream&);
        bool __identifier(LexerStream&, std::vector<std::string>&);

        bool __type(LexerStream&, std::vector<Type>&);
        bool __comma_list_type(LexerStream&, std::vector<Type>&, Lexer::token_t);
        bool __comma_list_type(LexerStream&, std::vector<Type>&, Lexer::token_t, const char*);

    public:

        Compiler();
        void Parse(const std::string&);

    };

}

#endif//__LOOM_COMPILER_HPP__