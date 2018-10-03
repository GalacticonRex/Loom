#ifndef __LOOM_PARSE_STREAM_HPP__
#define __LOOM_PARSE_STREAM_HPP__

#include "lexer.hpp"
#include <stack>

namespace Loom {

    struct LexerStream {
    private:

        Lexer& _src;
        uint32 _index;
        uint32 _depth;
        std::stack<uint32> _index_stack;

    public:

        LexerStream(Lexer&);

        const Lexer& getInternal() const;

        const Lexer::TokenHeader& front() const;
        const char* frontData() const;

        uint32 index() const;

        bool good();

        bool revert(uint32 i);
        
        bool next();
        bool prev();

        void push();
        bool pop();
        bool pop_stay();

        void reset();

    };

}

#endif//__LOOM_PARSE_STREAM_HPP__