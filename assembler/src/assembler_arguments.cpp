#include "assembler.hpp"
#include "assembler_helpers.hpp"
#include "util.hpp"
#include "version.hpp"
#include <cstring>
#include <cmath>
#include <fstream>
#include <set>

namespace Loom {

    Unit Object::__prefix_value() {
        need(Lexer::token_t::PREFIX, _stream);
        std::string name = _stream.frontData();

        Unit result = 1;

        if ( name == "unit" )
            result = sizeof(Unit);
        else if ( name == "address" )
            result = sizeof(void*);
        else if ( name == "sync" )
            result = sizeof(CODE::Sync*);
        else
            sendError("Invalid @prefix '%s' (%d:%d)",
                name.c_str(),
                _stream.front().line,
                _stream.front().col
                );

        _stream.next();

        if ( want(Lexer::token_t::OPEN_BRACKET, _stream) ) {
            _stream.next();

            Unit output;
            parse_value(_stream, output, PARSE_INT);
            result *= output;
            
            need(Lexer::token_t::CLOSE_BRACKET, _stream);
            _stream.next();
        }

        return result;
    }
    void Object::__arg_assign(Code type, Unit data, Unit loc) {
        __text_write_argument(_text, type, data, loc);
    }
    void Object::__arg_reference(Code type, Code allowed, const std::string& name, Unit loc) {
        __text_write_argument(_text, type, 0, loc);
        _needs_reference[loc] = name;
        _allowed[loc] = allowed;
    }
    void Object::__arg_prefix(Unit index) {
        __arg_assign(CODE::VALUE, __prefix_value(), index);
    }
    void Object::__arg_symbol(Unit index) {
        need(Lexer::token_t::SYMBOL, ":", _stream);
        _stream.next();
        need(Lexer::token_t::SYMBOL, ":", _stream);
        _stream.next();

        need(Lexer::token_t::TOKEN, _stream);
        std::string name = _stream.frontData();
        _stream.next();

        if ( _in_kernel_mode ) {
            _kernel_calls.push_back({
                name,
                _stream.front().line,
                _stream.front().col
            });
        }

        __arg_reference(ASM_TYPE_TEXT, ASM_TYPE_TEXT, name, index);
    }
    void Object::__arg_token(Unit index) {
        std::string name = _stream.frontData();
        __arg_reference(ASM_TYPE_VALUE, ASM_TYPE_VALUE | ASM_TYPE_DATA, name, index);
        _stream.next();
    }
    void Object::__arg_literal(Unit index) {
        __arg_assign(CODE::VALUE, __get_token_value(), index);
    }
    void Object::__arg_string(Unit index) {
        std::string variable = __get_anon_varname();
        std::string str = _stream.frontData();
        _stream.next();

        __references_t__::iterator find_string =
            _string_map.find(str);
        if ( find_string == _string_map.end() ) {
            _string_map.insert(find_string, std::make_pair(str, _data.size()));
            __set_reference(variable, _data.size());

            for ( const char* i=str.c_str();
                  *i != '\0'; i++ ) {
                _data.push_back(*i);
            }
            _data.push_back('\0');
        } else {
            __set_reference(variable, find_string -> second);
        }
        __set_type(variable, ASM_TYPE_DATA);

        __arg_reference(ASM_TYPE_DATA, ASM_TYPE_DATA, variable, index);
    }
    void Object::__arg_paran(Unit index) {
        _stream.next();
        switch( _stream.front().tokt ) {
            case Lexer::token_t::PREFIX:
                __arg_assign(CODE::LOCAL, __prefix_value(), index);
                break;
            case Lexer::token_t::TOKEN:
                if ( __token_is_variable() ) {
                    std::string name = _stream.frontData();
                    __arg_reference(ASM_TYPE_LOCAL, ASM_TYPE_VALUE, name, index);
                    _stream.next();
                } else
                    __arg_assign(CODE::LOCAL, __get_token_value(), index);
                break;
            default:
                sendError("Invalid internal %s '%s' for local (%d:%d)",
                    Lexer::token(_stream.front().tokt),
                    _stream.frontData(),
                    _stream.front().line,
                    _stream.front().col
                    );
        }
        need(Lexer::token_t::CLOSE_PARAN, _stream);
        _stream.next();
    }
    void Object::__arg_scope(Unit index) {
        _stream.next();
        switch( _stream.front().tokt ) {
            case Lexer::token_t::PREFIX:
                __arg_assign(CODE::ADDR, __prefix_value(), index);
                break;
            case Lexer::token_t::TOKEN:
                if ( __token_is_variable() ) {
                    std::string name = _stream.frontData();
                    __arg_reference(ASM_TYPE_ADDR, ASM_TYPE_VALUE, name, index);
                    _stream.next();
                } else
                    __arg_assign(CODE::ADDR, __get_token_value(), index);
                break;
            default:
                sendError("Invalid internal %s '%s' for address (%d:%d)",
                    Lexer::token(_stream.front().tokt),
                    _stream.frontData(),
                    _stream.front().line,
                    _stream.front().col
                    );
        }
        need(Lexer::token_t::CLOSE_SCOPE, _stream);
        _stream.next();
    }
    void Object::__arg_bracket(Unit index) {
        _stream.next();
        switch( _stream.front().tokt ) {
            case Lexer::token_t::PREFIX:
                __arg_assign(CODE::GLOB, __prefix_value(), index);
                break;
            case Lexer::token_t::TOKEN:
                if ( __token_is_variable() ) {
                    std::string name = _stream.frontData();
                    __arg_reference(ASM_TYPE_GLOB, ASM_TYPE_VALUE, name, index);
                    _stream.next();
                } else
                    __arg_assign(CODE::GLOB, __get_token_value(), index);
                break;
            default:
                sendError("Invalid internal %s '%s' for global (%d:%d)",
                    Lexer::token(_stream.front().tokt),
                    _stream.frontData(),
                    _stream.front().line,
                    _stream.front().col
                    );
        }
        need(Lexer::token_t::CLOSE_BRACKET, _stream);
        _stream.next();
    }

    void Object::__argument(const char* opcode, uint32 slot, Code allowed) {
        Lexer::token_t type = _stream.front().tokt;

        Unit code_loc = _text.size();
        __text_argument(0, 0);

        switch( type ) {
            case Lexer::token_t::PREFIX:
                if ( !(allowed & ASM_TYPE_VALUE) )
                    sendError("Cannot use @type for '%s' argument %d (%d:%d)",
                        opcode,
                        slot,
                        _stream.front().line,
                        _stream.front().col
                        );
                __arg_prefix(code_loc);
                break;
            case Lexer::token_t::STRING:
                if ( !(allowed & ASM_TYPE_DATA) )
                    sendError("Cannot use \"...\" for '%s' argument %d (%d:%d)",
                        opcode,
                        slot,
                        _stream.front().line,
                        _stream.front().col
                        );

                __arg_string(code_loc);
                break;
            case Lexer::token_t::SYMBOL:
                if ( !(allowed & ASM_TYPE_TEXT) )
                    sendError("Cannot use ::label for '%s' argument %d (%d:%d)",
                        opcode,
                        slot,
                        _stream.front().line,
                        _stream.front().col
                        );
                __arg_symbol(code_loc);
                break;
            case Lexer::token_t::TOKEN:
                if ( !(allowed & ASM_TYPE_VALUE) )
                    sendError("Cannot use VALUE for '%s' argument %d (%d:%d)",
                        opcode,
                        slot,
                        _stream.front().line,
                        _stream.front().col
                        );
                if ( __token_is_variable() ) 
                    __arg_token(code_loc);
                else
                    __arg_literal(code_loc);
                break;
            case Lexer::token_t::OPEN_PARAN:
                if ( !(allowed & ASM_TYPE_LOCAL) )
                    sendError("Cannot use STACK VALUE for '%s' argument %d (%d:%d)",
                        opcode,
                        slot,
                        _stream.front().line,
                        _stream.front().col
                        );
                __arg_paran(code_loc);
                break;
            case Lexer::token_t::OPEN_SCOPE:
                if ( !(allowed & ASM_TYPE_ADDR) )
                    sendError("Cannot use ADDRESS for '%s' argument %d (%d:%d)",
                        opcode,
                        slot,
                        _stream.front().line,
                        _stream.front().col
                        );
                __arg_scope(code_loc);
                break;
            case Lexer::token_t::OPEN_BRACKET:
                if ( !(allowed & ASM_TYPE_GLOB) )
                    sendError("Cannot use GLOBAL VALUE for '%s' argument %d (%d:%d)",
                        opcode,
                        slot,
                        _stream.front().line,
                        _stream.front().col
                        );
                __arg_bracket(code_loc);
                break;
            default:
                sendError("Invalid %s '%s' (%d:%d)",
                        Lexer::token(_stream.front().tokt),
                        _stream.frontData(),
                        _stream.front().line,
                        _stream.front().col
                        );
        }
    }

}