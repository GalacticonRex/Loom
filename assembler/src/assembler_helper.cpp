#include "assembler_helpers.hpp"
#include "util.hpp"
#include "assembler.hpp"
#include <cstring>
#include <cmath>

namespace Loom {

void need(Lexer::token_t t, const char* name, LexerStream& toks) {
    need(t, toks);
    if ( strcmp(toks.frontData(), name) != 0 ) {
        sendError("Got %s '%s' expected '%s' at (%d:%d)",
                        Lexer::token(toks.front().tokt),
                        toks.frontData(),
                        name,
                        toks.front().line,
                        toks.front().col);
    }
}
void need(Lexer::token_t t, LexerStream& toks) {
    if ( !toks.good() )
        sendError("Script ended prematurely, expected %s", Lexer::token(t));

    if ( toks.front().tokt != t ) {
        if ( *toks.frontData() != '\0' ) {
            sendError("Got %s %s expected %s at (%d:%d)",
                        Lexer::token(toks.front().tokt),
                        toks.frontData(),
                        Lexer::token(t),
                        toks.front().line,
                        toks.front().col);
        } else {
            sendError("Got %s expected %s at (%d:%d)",
                        Lexer::token(toks.front().tokt),
                        Lexer::token(t),
                        toks.front().line,
                        toks.front().col);
        }
    }
}
bool want(Lexer::token_t t, const char* cc, LexerStream& toks) {
    return ( want(t, toks) && strcmp(toks.frontData(), cc) == 0 );
}
bool want(Lexer::token_t t, LexerStream& toks) {
    return ( toks.good() && toks.front().tokt == t );
}

bool parse_int_hex(const char* data, uint64& output) {
    output = 0;
    for (;*data!='\0';++data) {
        output *= 16;
        if ( *data >= 'a' && *data <= 'f' ) {
            output += *data - 'a' + 10;
        } else if ( *data >= 'A' && *data <= 'F' ) {
            output += *data - 'A' + 10;
        } else if ( *data >= '0' && *data <= '9' ) {
            output += *data - '0';
        } else {
            return false;
        }
    }
    return true;
}
bool parse_int_oct(const char* data, uint64& output) {
    output = 0;
    for (;*data!='\0';++data) {
        output *= 8;
        if ( *data >= '0' && *data <= '7' ) {
            output += *data - '0';
        } else {
            return false;
        }
    }
    return true;
}
bool parse_int_dec(const char* data, uint64& output) {
    output = 0;
    for (;*data!='\0';++data) {
        output *= 10;
        if ( *data >= '0' && *data <= '9' ) {
            output += *data - '0';
        } else {
            return false;
        }
    }
    return true;
}
bool parse_int(const char* data, uint64& output) {
    if ( *data == '0' ) {
        ++ data;

        // hexdecimal
        if ( *data == 'x' ) {
            ++data;
            return parse_int_hex(data, output);
        }
        // octagonal
        else if ( *data == 't' ) {
            ++data;
            return parse_int_oct(data, output);
        }
        // decimal
        else {
            return parse_int_dec(data, output);
        }
    }
    // decimal
    else {
        return parse_int_dec(data, output);
    }
}

bool parse_float(const char* whole, const char* partial, float64& output) {
    uint64 lhs, rhs;
    if ( !parse_int_dec(whole, lhs) )
        return false;
    if ( !parse_int_dec(partial, rhs) )
        return false;
    output = (float64)lhs + (float64)rhs / std::pow(10.0, (float64)strlen(partial));
    return true;
}

void parse_value(LexerStream& toks, Unit& output, uint32 type) {
    bool negative = false;
    std::string total, lhs, rhs;
    uint32 ln = toks.front().line;
    uint32 col = toks.front().col;

    if ( want(Lexer::token_t::SYMBOL, "-", toks) ) {
        toks.next();
        total += '-';
        negative = true;
    }

    need(Lexer::token_t::TOKEN, toks);
    lhs = toks.frontData();
    total += lhs;
    toks.next();
    
    if ( want(Lexer::token_t::SYMBOL, ".", toks) && (type&Object::PARSE_FLOAT) ) {
        float64& f = *((float64*) &output);
        toks.next();
        if ( want(Lexer::token_t::TOKEN, toks) ) {
            rhs = toks.frontData();
            toks.next();
            total += "." + rhs;
            if ( !parse_float(lhs.c_str(), rhs.c_str(), f) ) {
                sendError("Invalid floating point value '%s' (%d:%d)", total.c_str(), ln, col);
            }
        } else {
            if ( !parse_float(lhs.c_str(), "", f) ) {
                sendError("Invalid floating point value '%s' (%d:%d)", total.c_str(), ln, col);
            }
        }
        if ( negative ) 
            f = -f;
    } else if ( type & Object::PARSE_INT ){
        int64& i = *((int64*) &output);
        uint64 ui;
        if ( !parse_int(lhs.c_str(), ui) ) {
            sendError("Invalid integer value '%s' (%d:%d)", total.c_str(), ln, col);
        }
        i = ( negative ) ? -(int64)ui : (int64)ui; 
    } else {
        sendError("Invalid value starting with'%s' (%d:%d)", total.c_str(), ln, col);
    }
}

}