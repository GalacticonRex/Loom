#ifndef __LOOM_VERIFY_SYNTAX_HPP__
#define __LOOM_VERIFY_SYNTAX_HPP__

#include "lexer_stream.hpp"
#include <vector>

namespace Loom {
    struct Syntax {

        enum FSM_STATE {
            S_START = 0,
            S_FINISH,

            S_IMPORT_STMT,
            S_IMPORT_NAME,
            S_IMPORT_DOT,
            S_IMPORT_STAR,

            S_STMT_LTOKEN,
            S_STMT_LTYPE,
        };

    private:

        LexerStream& _stream;
        std::vector<FSM_STATE> _path;
        std::vector<const char*> _data;

    public:

        bool s_start();
        bool s_import_stmt();
        bool s_import_name();
        bool s_import_dot();
        bool s_import_star();

        Syntax(LexerStream& stream);
        const std::vector<FSM_STATE>& getPath() const;
        const std::vector<const char*>& getData() const;
        std::pair<FSM_STATE, const char*> getElement(uint32) const;

        bool statement();

    };
}

#endif//__LOOM_VERIFY_SYNTAX_HPP__
