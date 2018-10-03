#include "verify_syntax.hpp"
#include <cstring>
#include <iostream>

namespace Loom {

    Syntax::Syntax(LexerStream& stream) :
        _stream(stream) { ; }
    const std::vector<Syntax::FSM_STATE>& Syntax::getPath() const {
        return _path;
    }
    const std::vector<const char*>& Syntax::getData() const {
        return _data;
    }
    std::pair<Syntax::FSM_STATE, const char*> Syntax::getElement(uint32 i) const {
        if ( i >= _path.size() )
            sendError("Cannot access element %d out of %d", i, _path.size());
        return std::make_pair(_path[i], _data[i]);
    }
    bool Syntax::statement() {
        bool ret;

        _path.push_back(S_START);
        _data.push_back(nullptr);

        for(;;) {
            FSM_STATE st = _path.back();
            switch(st) {
                case S_FINISH:          return ret;
                case S_START:           ret = s_start();            break;
                case S_IMPORT_STMT:     ret = s_import_stmt();      break;
                case S_IMPORT_NAME:     ret = s_import_name();      break;
                case S_IMPORT_DOT:      ret = s_import_dot();      break;
                case S_IMPORT_STAR:     ret = s_import_star();      break;
            }
            if ( !ret ) {
                std::fprintf(stderr, "Syntax Error at (%d:%d)\n", _stream.front().line, _stream.front().col);
                return false;
            } else {
                _stream.next();
            }
        }
    }
    bool Syntax::s_start() {
        switch( _stream.front().tokt ) {
            case Lexer::token_t::PREFIX:
                if ( std::strcmp(_stream.frontData(), "import") == 0 )
                    _path.push_back(S_IMPORT_STMT);
                else if ( std::strcmp(_stream.frontData(), "main") == 0 )
                    return false; // ...
                else
                    return false;
                break;
            case Lexer::token_t::TOKEN:
                break;
            default:
                return false;
        }
        return true;
    }
    bool Syntax::s_import_stmt() {
        switch( _stream.front().tokt ) {
            case Lexer::token_t::TOKEN:
                _path.push_back(S_IMPORT_NAME);
                _data.push_back(_stream.frontData());
                break;
            default:
                return false;
        }
        return true;
    }
    bool Syntax::s_import_name() {
        switch( _stream.front().tokt ) {
            case Lexer::token_t::SYMBOL: {
                if ( std::strcmp(_stream.frontData(), ";") == 0 )
                    _path.push_back(S_FINISH);
                else if ( std::strcmp(_stream.frontData(), ".") == 0 )
                    _path.push_back(S_IMPORT_DOT);
                else if ( std::strcmp(_stream.frontData(), "*") == 0 )
                    _path.push_back(S_IMPORT_STAR);
                else
                    return false;
            } break;
            default:
                return false;
        }
        return true;
    }
    bool Syntax::s_import_dot() {
        switch( _stream.front().tokt ) {
            case Lexer::token_t::TOKEN:
                _path.push_back(S_IMPORT_NAME);
                _data.push_back(_stream.frontData());
                break;
            default:
                return false;
        }
        return true;
    }
    bool Syntax::s_import_star() {
        switch( _stream.front().tokt ) {
            case Lexer::token_t::SYMBOL:
                if ( std::strcmp(_stream.frontData(), ";") == 0 )
                    _path.push_back(S_FINISH);
                else
                    return false;
                break;
            default:
                return false;
        }
        return true;
    }

}
