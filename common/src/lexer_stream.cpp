#include "lexer_stream.hpp"
#include <cstdio>

namespace Loom {

    static Lexer::TokenHeader _terminate = {Lexer::token_t::NONE, -1, 0, 0};

    LexerStream::LexerStream(Lexer& src) :
        _src(src), _index(0) { ; }

    const Lexer& LexerStream::getInternal() const {
        return _src;
    }

    const Lexer::TokenHeader& LexerStream::front() const {
        while ( _index >= _src.tokenCount() )
            if ( !_src.generate() )
                return _terminate;

        return _src.header(_index);
    }
    const char* LexerStream::frontData() const {
        while ( _index >= _src.tokenCount() )
            if ( !_src.generate() )
                return "";
        
        return _src.data(_index);
    }

    bool LexerStream::good() {
        return (_index < _src.tokenCount() || _src.good());
    }

    uint32 LexerStream::index() const {
        return _index;
    }
    bool LexerStream::revert(uint32 i) {
        if ( i >= _src.tokenCount() )
            return false;
        _index = i;
        return true;
    }

    bool LexerStream::next() {
        if ( good() )
            ++ _index;
        return good();
    }
    bool LexerStream::prev() {
        if ( _index == 0 )
            return false;
        -- _index;
        return true;
    }
    void LexerStream::push() {
        _index_stack.push(_index);
    }
    bool LexerStream::pop() {
        if ( _index_stack.empty() )
            return false;
        _index = _index_stack.top();
        _index_stack.pop();
        return true;
    }
    bool LexerStream::pop_stay() {
        if ( _index_stack.empty() )
            return false;
        _index_stack.pop();
        return true;
    }
    void LexerStream::reset() {
        _index = 0;
    }

}