#ifndef __LOOM_ASSEMBLER_HELPERS_HPP__
#define __LOOM_ASSEMBLER_HELPERS_HPP__

#include "types.hpp"
#include "code.hpp"
#include "lexer_stream.hpp"

namespace Loom {

extern void need(Lexer::token_t, const char*, LexerStream& toks);
extern void need(Lexer::token_t, LexerStream& toks);
extern bool want(Lexer::token_t, const char*, LexerStream& toks);
extern bool want(Lexer::token_t, LexerStream& toks);
extern bool parse_int_hex(const char* data, uint64& output);
extern bool parse_int_oct(const char* data, uint64& output);
extern bool parse_int_dec(const char* data, uint64& output);
extern bool parse_int(const char* data, uint64& output);
extern bool parse_float(const char* whole, const char* partial, float64& output);
extern void parse_value(LexerStream& toks, Unit& output, uint32 type);

}

#endif//__LOOM_ASSEMBLER_HELPERS_HPP__