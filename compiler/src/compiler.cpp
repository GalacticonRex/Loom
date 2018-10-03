/*#include "compiler.hpp"
#include <cstring>
#include <iostream>
#include <iomanip>

namespace Loom {

    const Compiler::Type Compiler::VAR_START   = 0x10;
    const Compiler::Type Compiler::VAR_END     = 0x11;
    const Compiler::Type Compiler::TYPE_START  = 0x20;
    const Compiler::Type Compiler::TYPE_END    = 0x21;
    const Compiler::Type Compiler::ARRAY_START = 0x30;
    const Compiler::Type Compiler::ARRAY_END   = 0x31;
    const Compiler::Type Compiler::FUNC_INPUT  = 0x40;
    const Compiler::Type Compiler::FUNC_OUTPUT = 0x41;
    const Compiler::Type Compiler::FUNC_END    = 0x42;
    const Compiler::Type Compiler::TUPLE_START = 0x50;
    const Compiler::Type Compiler::TUPLE_END   = 0x51;

    static void errorAtLocation(const char* msg, LexerStream& toks);
    static void need(const char*, Lexer::token_t, const char*, LexerStream& toks);
    static void need(const char*, Lexer::token_t, LexerStream& toks);
    static bool want(Lexer::token_t, const char*, LexerStream& toks);
    static bool want(Lexer::token_t, LexerStream& toks);

    bool Compiler::__textile(LexerStream& toks) {
        if ( !want(Lexer::token_t::TOKEN, "textile", toks) )
            return false;
        
        toks.next();

        need("Please provide a name for this textile.", Lexer::token_t::TOKEN, toks);

        std::string name = toks.frontData();
        _namespace.push(name);
        _namespace_type.push(ScopeType::TEXTILE);
        std::cerr << "Entering TEXTILE '" << name << "'" << std::endl;

        toks.next();

        need("Must open a textile with { ... }", Lexer::token_t::OPEN_SCOPE, toks);
        toks.next();

        return true;
    };

    bool Compiler::__expr(LexerStream& toks, std::vector<Type>& expr) {

        if ( want(Lexer::token_t::SYMBOL, ":", toks) ) {
            // this is a type decl
            std::vector<Type> to_type;
            if ( want(Lexer::token_t::TOKEN, toks) ) {
                toks.next();
                if ( want(Lexer::token_t::SYMBOL, "->", toks) ) {
                    // is a function
                } else
                    toks.prev();
            }

            if ( !__expr(toks, to_type) )
                errorAtLocation("Expected an instance to type", toks);
        }

        return false;
    }

    bool Compiler::__function_args(LexerStream& toks) {
        return false;
    }

    bool Compiler::__statement(LexerStream& toks) {
        if ( want(Lexer::token_t::CLOSE_SCOPE, toks) ) {
            
            if ( _namespace.empty() )
                errorAtLocation("Unpaired '}' here.", toks);

            _namespace.pop();
            _namespace_type.pop();

        } else if ( __textile(toks) ) {

            // textile statement

        } else if ( __import(toks ) ) {

            // import statement

        } else {
            need("Statement must have an identifier.", Lexer::token_t::TOKEN, toks);
            std::string stmt_name = toks.frontData();
            toks.next();

            std::unordered_map<std::string, Instance>::iterator iter = _inst_map.find(stmt_name);

            if ( want(Lexer::token_t::SYMBOL, ":", toks) ) {
                toks.next();

                if ( !want(Lexer::token_t::SYMBOL, "=", toks) ) {
                    std::vector<Type> type_def;
                    __type(toks, type_def);

                    if ( want(Lexer::token_t::END_STMT, toks) ) {
                        toks.next();
                        return true;
                    }
                }

                toks.next();
                std::vector<Type> moo;
                __expr(toks, moo);
                need("Must terminate an expression with a ';'", Lexer::token_t::END_STMT, toks);
                toks.next();

                _inst_map[stmt_name] = Instance();
            } else if ( want(Lexer::token_t::SYMBOL, "->", toks) ) {
                errorAtLocation("Function pattern matching not implemented!", toks);
                // function pattern matching!
            } else if ( want(Lexer::token_t::OPEN_PARAN, toks) ) {
                errorAtLocation("Function invocation not implemented!", toks);
                // function invocation!
            } else {
                errorAtLocation("Statement must either be a declaration or an invocation to be valid.", toks);
            }
        }

        return true;
    }

    bool Compiler::__import(LexerStream& toks) {
        if ( !want(Lexer::token_t::TOKEN, "import", toks) )
            return false;

        toks.next();

        std::vector<std::string> names;

        if ( !__identifier(toks, names) ) {
            errorAtLocation("Import statement must have an identifier to import.", toks);
        }

        if ( want(Lexer::token_t::SYMBOL, "*", toks)) {
            toks.next();

            // import in-place

        } else {

            // import with name

        }

        need("Import statement does not terminate.", Lexer::token_t::END_STMT, toks);
        toks.next();

        return true;
    }

    bool Compiler::__identifier(LexerStream& toks, std::vector<std::string>& names) {
        if ( want(Lexer::token_t::TOKEN, toks) ) {
            names.push_back(toks.frontData());
            toks.next();
            while ( want(Lexer::token_t::SYMBOL, ".", toks) ) {
                toks.next();
                need("Unresolved \".\" in identifier.", Lexer::token_t::TOKEN, toks);
                names.push_back(toks.frontData());
                toks.next();
            }
            return true;
        } else {
            return false;
        }
    }

    bool Compiler::__type(LexerStream& toks, std::vector<Type>& type) {
        // Token Decl
        if ( want(Lexer::token_t::TOKEN, toks) ) {
            type.push_back(VAR_START);
            std::vector<std::string> names;
            __identifier(toks, names);
            type.push_back(VAR_END);
        }

        // Tuple Decl
        else if ( want(Lexer::token_t::OPEN_PARAN, toks) ) {
            toks.next();

            type.push_back(TUPLE_START);
            __comma_list_type(toks, type, Lexer::token_t::CLOSE_PARAN);
            type.push_back(TUPLE_END);

            need("Close Tuple Declaration", Lexer::token_t::CLOSE_PARAN, toks);
            toks.next();
        }

        // Array Decl
        else if ( want(Lexer::token_t::OPEN_BRACKET, toks) ) {
            toks.next();

            type.push_back(ARRAY_START);
            if ( !__type(toks, type) )
                errorAtLocation("Looking for a type declaration but did not find one.", toks);
            type.push_back(ARRAY_END);

            need("Close Array Declaration", Lexer::token_t::CLOSE_BRACKET, toks);
            toks.next();
        }

        // Type Decl
        else if ( want(Lexer::token_t::SYMBOL, ":", toks) ) {
            toks.next();

            type.push_back(TYPE_START);
            need("Need a single token name for a type reference", Lexer::token_t::TOKEN, toks);
            std::vector<std::string> names;
            __identifier(toks, names);
            type.push_back(TYPE_END);
        }

        // Function Decl
        else if ( want(Lexer::token_t::OPEN_SCOPE, toks) ) {
            toks.next();

            type.push_back(FUNC_INPUT);
            __comma_list_type(toks, type, Lexer::token_t::SYMBOL, "->");

            need("A function declaration must have an output section (even if it is empty)", Lexer::token_t::SYMBOL, "->", toks);
            toks.next();

            type.push_back(FUNC_OUTPUT);

            __comma_list_type(toks, type, Lexer::token_t::CLOSE_SCOPE);

            type.push_back(FUNC_END);

            need("Need to close a function scope after opening it.", Lexer::token_t::CLOSE_SCOPE, toks);
            toks.next();
        }

        // Invalid syntax
        else {
            return false;
        }

        return true;
    }

    bool Compiler::__comma_list_type(LexerStream& toks, std::vector<Type>& type_def, Lexer::token_t ttype) {
        bool ret = false;
        bool comma_sep = false;
        int length = 0;
        while ( __type(toks, type_def) ) {
            if ( want(ttype, toks) ) {
                ret = true;
                break;
            } else if ( comma_sep ) {
                need("Expected a comma to separate items in this list.", Lexer::token_t::SYMBOL, ",", toks);
                toks.next();
            } else if ( want(Lexer::token_t::SYMBOL, ",", toks) ) {
                if ( ret ) {
                    errorAtLocation("Cannot comma separate a non-comma separated list. Entire list must be comma separated.", toks);
                }
                comma_sep = true;
                toks.next();
            }

            ret = true;
            length ++ ;
        }
        return ret;
    }
    bool Compiler::__comma_list_type(LexerStream& toks, std::vector<Type>& type_def, Lexer::token_t ttype, const char* terminal) {
        bool ret = false;
        bool comma_sep = false;
        int length = 0;
        while ( __type(toks, type_def) ) {
            if ( want(ttype, terminal, toks) ) {
                ret = true;
                break;
            } else if ( comma_sep ) {
                need("Expected a comma to separate items in this list.", Lexer::token_t::SYMBOL, ",", toks);
                toks.next();
            } else if ( want(Lexer::token_t::SYMBOL, ",", toks) ) {
                if ( ret ) {
                    errorAtLocation("Cannot comma separate a non-comma separated list. Entire list must be comma separated.", toks);
                }
                comma_sep = true;
                toks.next();
            }

            ret = true;
            length ++ ;
        }
        return ret;
    }

    Compiler::Compiler() { ; }

    void Compiler::Parse(const std::string& loom) {
        Lexer p(loom);

        p.addOperator("*");
        p.addOperator(":");
        p.addOperator("=");
        p.addOperator("|");
        p.addOperator(",");
        p.addOperator(".");
        p.addOperator("->");
        p.addOperator("<-");
        p.addOperator("@");
        p.addOperator("?");

        Loom::LexerStream ps(p);
        while (ps.good()) {
            __statement(ps);
        }
    }

    static void errorAtLocation(const char* msg, LexerStream& toks) {
        if ( *toks.frontData() != '\0' ) {
            sendError("Invalid %s '%s' at (%d:%d) ==> \n%s",
                Lexer::token(toks.front().tokt),
                toks.frontData(),
                toks.front().line,
                toks.front().col,
                msg);
        } else {
            sendError("Invalid %s at (%d:%d) ==> \n%s",
                Lexer::token(toks.front().tokt),
                toks.front().line,
                toks.front().col,
                msg);
        }
    }

    static void need(const char* msg, Lexer::token_t t, const char* name, LexerStream& toks) {
        if ( !toks.good() )
            sendError("Script ended prematurely, expected %s '%s'", Lexer::token(t), name);

        if ( toks.front().tokt != t ) {
            if ( *toks.frontData() != '\0' ) {
                sendError("Got %s '%s' expected %s '%s' at (%d:%d) ==> \n%s",
                            Lexer::token(toks.front().tokt),
                            toks.frontData(),
                            Lexer::token(t),
                            name,
                            toks.front().line,
                            toks.front().col,
                            msg);
            } else {
                sendError("Got '%s' expected %s '%s' at (%d:%d) ==> \n%s",
                            Lexer::token(toks.front().tokt),
                            Lexer::token(t),
                            name,
                            toks.front().line,
                            toks.front().col,
                            msg);
            }
        }
        if ( strcmp(toks.frontData(), name) != 0 ) {
            sendError("Got %s '%s' expected %s '%s' at (%d:%d) ==> \n%s",
                            Lexer::token(toks.front().tokt),
                            toks.frontData(),
                            Lexer::token(t),
                            name,
                            toks.front().line,
                            toks.front().col,
                            msg);
        }
    }
    static void need(const char* msg, Lexer::token_t t, LexerStream& toks) {
        if ( !toks.good() )
            sendError("Script ended prematurely, expected '%s'", Lexer::token(t));

        if ( toks.front().tokt != t ) {
            if ( *toks.frontData() != '\0' ) {
                sendError("Got %s '%s' expected %s at (%d:%d) ==> \n%s",
                            Lexer::token(toks.front().tokt),
                            toks.frontData(),
                            Lexer::token(t),
                            toks.front().line,
                            toks.front().col,
                            msg);
            } else {
                sendError("Got '%s' expected %s at (%d:%d) ==> \n%s",
                            Lexer::token(toks.front().tokt),
                            Lexer::token(t),
                            toks.front().line,
                            toks.front().col,
                            msg);
            }
        }
    }
    static bool want(Lexer::token_t t, const char* cc, LexerStream& toks) {
        return ( want(t, toks) && strcmp(toks.frontData(), cc) == 0 );
    }
    static bool want(Lexer::token_t t, LexerStream& toks) {
        return ( toks.good() && toks.front().tokt == t );
    }

}*/