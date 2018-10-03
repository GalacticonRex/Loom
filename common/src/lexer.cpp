#include "util.hpp"
#include "lexer.hpp"
#include <string.h>
#include <iostream>

namespace Loom {

    const char* Lexer::token(token_t t) {
        switch (t) {
            case token_t::NONE: return "none";
            case token_t::TOKEN: return "token";
            case token_t::SYMBOL: return "symbol";
            case token_t::STRING: return "string";
            case token_t::PREFIX: return "prefix";
            case token_t::OPEN_PARAN: return "(";
            case token_t::CLOSE_PARAN: return ")";
            case token_t::OPEN_BRACKET: return "[";
            case token_t::CLOSE_BRACKET: return "]";
            case token_t::OPEN_SCOPE: return "{";
            case token_t::CLOSE_SCOPE: return "}";
            default:
                return "unknown";
        }
    }

    Lexer::__stream__::__stream__(const std::string& dat) :
        _source(dat), _data(_source.c_str()), _line(1), _col(1) { ; }

    bool Lexer::__stream__::good() const {
        return (*_data != '\0');
    }
    uint32 Lexer::__stream__::column() const {
        return _col;
    }
    uint32 Lexer::__stream__::line() const {
        return _line;
    }
    char Lexer::__stream__::operator*() const {
        return *_data;
    }
    const char* Lexer::__stream__::operator[](uint32 index) const {
        return _data + index;
    }
    const char* Lexer::__stream__::operator++() {
        if ( good() ) {
            ++ _data;
            if ( *_data == '\n' ) {
                _line ++ ;
                _col = 0;
            } else
                _col ++ ;
        }
        return _data;
    }
    const char* Lexer::__stream__::operator++(int _) {
        return ++ (*this);
    }

    void Lexer::__create_token(token_t t, uint32 ln, uint32 col, const char* dat) {
        if ( dat == nullptr ) {

            _headers.push_back({t, -1, ln, col});

        } else {

            uint32 length = strlen(dat) + 1;

            TokenHeader head = {t, (int32)_data_buffer.size(), ln, col};
            _headers.push_back(head);

            uint32 index = _data_buffer.size();
            _data_buffer.resize(index + length);

            strncpy(&_data_buffer[index], dat, length);

        }
    }
    void Lexer::__export_token(token_t t, const char* orig, const char* termin, uint32 ln, uint32 col) {
        uint32 len = termin - orig;
        char buffer[len+1];
        memcpy(buffer, orig, len);
        buffer[len] = '\0';

        (void) __create_token(t, ln, col, buffer);
    }
    void Lexer::__generate_token(__stream__& dat) {
        while( dat.good() ) {
            if (*dat == '"' || *dat == '\'') {
                if ( __process_string(dat) )
                    return;
            } else if ( isPrefix(*dat) ) {
                if ( __process_prefix(dat) )
                    return;
            } else if ( isReserved(*dat) ) {
                if ( __process_reserved(dat) )
                    return;
            } else if ( isAlphaNumeric(*dat) ) {
                if ( __process_token(dat) )
                    return;
            } else if ( !isWhitespace(*dat) ) {
                if ( __process_operators(dat) )
                    return;
            } else
                ++ dat;
        }
    }
    bool Lexer::__process_comment(__stream__& dat) {
        for (;dat.good() && *dat!='\n';++dat);
        return true;
    }
    bool Lexer::__process_string(__stream__& dat) {
        uint32 ln = dat.line();
        uint32 col = dat.column();

        char src = *dat;
        const char* orig = ++ dat;

        for (;dat.good() && *dat!=src;++dat);
        if ( *dat == '\0' )
            sendError("Non-terminating string starting at (%d:%d)", ln, col); 

        uint32 slen = dat[0] - orig;
        char buffer[slen];
        char* buffer_head = buffer;
        for ( uint32 i=0;i<slen;i++,buffer_head++ ) {
            if ( orig[i] == '\\' ) {
                ++ i;
                switch( orig[i] ) {
                    case '\\': *buffer_head = '\\'; break;
                    case 'n': *buffer_head = '\n'; break;
                    case 'r': *buffer_head = '\r'; break;
                    case 't': *buffer_head = '\t'; break;
                    default:
                        sendError("Unknown escape code \\%c (%d:%d)", orig[i], ln, col);
                        break;
                }
            } else
                *buffer_head = orig[i];
        }

        __export_token(token_t::STRING, buffer, buffer_head, ln, col);
        ++ dat;
        return true;
    }
    bool Lexer::__process_prefix(__stream__& dat) {
        uint32 ln = dat.line();
        uint32 col = dat.column();

        ++ dat;
        const char* start = dat[0];

        for ( ;dat.good() && isAlphaNumeric(*dat);++dat);
        if ( dat[0] == start )
            sendError("Empty prefix (%d:%d)", ln, col);

        __export_token(token_t::PREFIX, start, dat[0], ln, col);
        return true;
    }
    bool Lexer::__process_reserved(__stream__& dat) {
        token_t type = (token_t)*dat;
        uint32 ln = dat.line();
        uint32 col = dat.column();

        static const char* _error_string = "Unmatched '%c' at (%d:%d), found '%c' at (%d:%d) instead";

        switch ( type ) {
            case token_t::OPEN_PARAN:
            case token_t::OPEN_BRACKET:
            case token_t::OPEN_SCOPE:
                _scope_stack.push({type, ln, col});
                break;
            case token_t::CLOSE_PARAN: {
                __scope__ s = _scope_stack.top();
                if ( s.type != token_t::OPEN_PARAN )
                    sendError(_error_string, (char)s.type, s.ln, s.col, *dat, ln, col);
                _scope_stack.pop();
            }   break;
            case token_t::CLOSE_BRACKET: {
                __scope__ s = _scope_stack.top();
                if ( s.type != token_t::OPEN_BRACKET )
                    sendError(_error_string, (char)s.type, s.ln, s.col, *dat, ln, col);
                _scope_stack.pop();
            }   break;
            case token_t::CLOSE_SCOPE: {
                __scope__ s = _scope_stack.top();
                if ( s.type != token_t::OPEN_SCOPE )
                    sendError(_error_string, (char)s.type, s.ln, s.col, *dat, ln, col);
                _scope_stack.pop();
            }   break;
            default:
                break;
        }

        __create_token(type, ln, col, nullptr);
        ++ dat;
        return true;
    }
    bool Lexer::__process_token(__stream__& dat) {
        const char* orig = dat[0];
        uint32 ln = dat.line();
        uint32 col = dat.column();

        for (;dat.good() &&
              isAlphaNumeric(*dat);++dat);

        __export_token(token_t::TOKEN, orig, dat[0], ln, col);
        return true;
    }
    bool Lexer::__process_operators(__stream__& dat) {
        const char* orig = dat[0];
        const char* termin = orig;
        uint32 ln = dat.line();
        uint32 col = dat.column();
        
        // check for comments
        bool comment = false;
        for (;dat.good() &&
              !isAlphaNumeric(*dat) &&
              !isWhitespace(*dat) &&
              !isReserved(*dat) &&
              !isPrefix(*dat);) {
            if (*dat == '/') {
                if ( comment ) {
                    termin = dat[0] - 1;
                    __process_comment(dat);
                    break;
                } else
                    comment = true;
            } else
                comment = false;
            ++ dat;
            termin = dat[0];
        }

        uint32 len = termin - orig;
        if ( len > 0 ) {
            char buffer[len+1];
            memcpy(buffer, orig, len);
            buffer[len] = '\0';
            const char* buffer_ptr = buffer;

            while ( *buffer_ptr != '\0') {
                uint32 amt = _operators.match(buffer_ptr);
                if ( amt == 0 )
                    sendError("Unknown operator %s (%d:%d)", buffer, ln, col);
                __export_token(token_t::SYMBOL, buffer_ptr, buffer_ptr+amt, ln ,col);
                buffer_ptr += amt;
                col += amt;
            }

            return true;
        } else {
            return false;
        }
    }

    Lexer::Lexer(const std::string& data) :
        _stream(data) { ; }

    bool Lexer::isPrefix(char c) const {
        for ( uint32 i=0;i<_prefixes.size();i++ )
            if ( _prefixes[i] == c )
                return true;
        return false;
    }
    bool Lexer::isReserved(char c) const {
        return ( c == (char)token_t::OPEN_PARAN || 
                 c == (char)token_t::CLOSE_PARAN ||
                 c == (char)token_t::OPEN_BRACKET ||
                 c == (char)token_t::CLOSE_BRACKET ||
                 c == (char)token_t::OPEN_SCOPE ||
                 c == (char)token_t::CLOSE_SCOPE );
    }
    bool Lexer::isWhitespace(char c) const {
        for ( uint32 i=0;i<_whitespace.size();i++ )
            if ( _whitespace[i] == c )
                return true;
        return false;
    }
    bool Lexer::isAlphaNumeric(char c) const {
        return (isAlpha(c) || isNumeric(c));
    }
    bool Lexer::isAlpha(char c) const {
        for ( uint32 i=0;i<_alphas.size();i++ )
            if ( c == _alphas[i] )
                return true;
        for ( uint32 i=0;i<_alpha_ranges.size();i++ )
            if ( c >= _alpha_ranges[i].first && c <= _alpha_ranges[i].second )
                return true;
        return false;
    }
    bool Lexer::isNumeric(char c) const {
        for ( uint32 i=0;i<_numerics.size();i++ )
            if ( c == _numerics[i] )
                return true;
        for ( uint32 i=0;i<_numeric_ranges.size();i++ )
            if ( c >= _numeric_ranges[i].first && c <= _numeric_ranges[i].second )
                return true;
        return false;
    }

    void Lexer::addAlphaRange(char start, char end) {
        for( char elem=start;elem<=end;elem++ ) {
            if ( isPrefix(elem) )
                sendError("Cannot add alphabetic range '%c' through '%c' because it contains a prefix", elem);
            if ( isReserved(elem) )
                sendError("Cannot add alphabetic range '%c' through '%c' because it contains a reserved character", elem);
            if ( isWhitespace(elem) )
                sendError("Cannot add alphabetic range '%c' through '%c' because it contains a whitespace character", elem);
            if ( isNumeric(elem) )
                sendError("Cannot add alphabetic range '%c' through '%c' because it contains a numeric character", elem);
            if ( isAlpha(elem) )
                sendError("Cannot add alphabetic range '%c' through '%c' because it already contains an alphabetic character", elem);
        }

        _alpha_ranges.push_back(std::make_pair(start, end));
    }
    void Lexer::addNumericRange(char start, char end) {
        for( char elem=start;elem<=end;elem++ ) {
            if ( isPrefix(elem) )
                sendError("Cannot add numeric range '%c' through '%c' because it contains a prefix", elem);
            if ( isReserved(elem) )
                sendError("Cannot add numeric range '%c' through '%c' because it contains a reserved character", elem);
            if ( isWhitespace(elem) )
                sendError("Cannot add numeric range '%c' through '%c' because it contains a whitespace character", elem);
            if ( isAlpha(elem) )
                sendError("Cannot add numeric range '%c' through '%c' because it contains an alphabetic character", elem);
            if ( isNumeric(elem) )
                sendError("Cannot add numeric range '%c' through '%c' because it already contains a numeric character", elem);
        }
        _numeric_ranges.push_back(std::make_pair(start, end));
    }
    void Lexer::addAlpha(char elem) {
        if ( isPrefix(elem) )
            sendError("Cannot add alphabetic '%c' because it is a prefix", elem);
        if ( isReserved(elem) )
            sendError("Cannot add alphabetic '%c' because it is a reserved character", elem);
        if ( isWhitespace(elem) )
            sendError("Cannot add alphabetic '%c' because it is a whitespace character", elem);
        if ( isNumeric(elem) )
            sendError("Cannot add alphabetic '%c' because it is a numeric character", elem);
        if ( isAlpha(elem) )
            sendError("Cannot add alphabetic '%c' because it is already an alphabetic character", elem);

        _alphas.push_back(elem);
    }
    void Lexer::addNumeric(char elem) {
        if ( isPrefix(elem) )
            sendError("Cannot add numeric '%c' because it is a prefix", elem);
        if ( isReserved(elem) )
            sendError("Cannot add numeric '%c' because it is a reserved character", elem);
        if ( isWhitespace(elem) )
            sendError("Cannot add numeric '%c' because it is a whitespace character", elem);
        if ( isAlpha(elem) )
            sendError("Cannot add numeric '%c' because it is an alphabetic character", elem);
        if ( isNumeric(elem) )
            sendError("Cannot add numeric '%c' because it is already a numeric", elem);

        _numerics.push_back(elem);
    }

    void Lexer::addOperator(const std::string& data) {
        for ( uint32 i=0;i<data.size();i++ ) {
            char c = data[i];
            if ( isPrefix(c) || isReserved(c) || isWhitespace(c) || isAlpha(c) || isNumeric(c) )
                sendError("Cannot add operator %s because it contains non-operator character %c", data.c_str(), c);
        }
        _operators.insert(data.c_str());
    }
    void Lexer::addPrefix(char prefix) { 
        if ( isPrefix(prefix) )
            sendError("Cannot add prefix '%c' because it is already a prefix", prefix);
        if ( isReserved(prefix) )
            sendError("Cannot add prefix '%c' because it is a reserved character", prefix);
        if ( isWhitespace(prefix) )
            sendError("Cannot add prefix '%c' because it is a whitespace character", prefix);
        if ( isAlphaNumeric(prefix) )
            sendError("Cannot add prefix '%c' because it is alpha-numeric", prefix);

        Trie::iterator iter = _operators.begin();
        do {
            if ( strchr(iter.get().c_str(), prefix) )
                sendError("Cannot add prefix '%c' because it is already part of operator '%s'", prefix, iter.get().c_str());
        } while ( iter.next() );

        _prefixes.push_back(prefix);
    }
    void Lexer::addWhitespace(char ws) {
        if ( isWhitespace(ws) )
            sendError("Cannot add whitespace '%c' because it is already a whitespace character", ws);
        if ( isPrefix(ws) )
            sendError("Cannot add whitespace '%c' because it is a prefix", ws);
        if ( isReserved(ws) )
            sendError("Cannot add whitespace '%c' because it is a reserved character", ws);
        if ( isAlphaNumeric(ws) )
            sendError("Cannot add whitespace '%c' because it is alpha-numeric", ws);

        Trie::iterator iter = _operators.begin();
        do {
            if ( strchr(iter.get().c_str(), ws) )
                sendError("Cannot add whitespace '%c' because it is already part of operator '%s'", ws, iter.get().c_str());
        } while ( iter.next() );

        _whitespace.push_back(ws);
    }

    bool Lexer::generate() {
        if ( _stream.good() ) {
            __generate_token(_stream);
            return true;
        }
        return false;
    }

    bool Lexer::good() const {
        return _stream.good();
    }
    uint32 Lexer::tokenCount() const {
        return _headers.size();
    }

    const Lexer::TokenHeader& Lexer::header(uint32 i) const {
        return _headers[i];
    }
    const char* Lexer::data(uint32 i) const {
        return (_headers[i].index == -1) ? "" : &(_data_buffer[_headers[i].index]);
    }
    uint32 Lexer::line(uint32 i) const {
        return _headers[i].line;
    }
    uint32 Lexer::column(uint32 i) const {
        return _headers[i].col;
    }

}