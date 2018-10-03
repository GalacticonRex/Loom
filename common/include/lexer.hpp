#ifndef __LOOM_PARSER_HPP__
#define __LOOM_PARSER_HPP__

#include <vector>
#include <stack>
#include <string>
#include "types.hpp"
#include "trie.hpp"
#include "util.hpp"

namespace Loom {

    struct Lexer {
        
        enum class token_t : unsigned char {
            NONE = '\0',
            TOKEN = 'a',
            SYMBOL = '+',
            STRING = '"',
            PREFIX = '@',
            OPEN_PARAN = '(',
            CLOSE_PARAN = ')',
            OPEN_BRACKET = '[',
            CLOSE_BRACKET = ']',
            OPEN_SCOPE = '{',
            CLOSE_SCOPE = '}'
        };

        static const char* token(token_t);
        
        struct TokenHeader {
            token_t tokt;
            int32 index;

            // debug info
            uint32 line;
            uint32 col;
        };

    private:

        struct __stream__ {
        private:
            
            std::string _source;
            const char* _data;
            uint32 _line;
            uint32 _col;

        public:

            __stream__(const std::string&);

            bool good() const;
            
            uint32 column() const;
            uint32 line() const;
            char operator*() const;
            const char* operator[](uint32) const;

            const char* operator++();
            const char* operator++(int _);
        };

        struct __scope__ {
            token_t type;
            uint32 ln;
            uint32 col;
        };
        
        __stream__ _stream;
        std::vector<TokenHeader> _headers;
        std::vector<char> _data_buffer;

        std::stack<__scope__> _scope_stack;
        std::vector<char> _alphas;
        std::vector<char> _numerics;
        std::vector<std::pair<char,char>> _alpha_ranges;
        std::vector<std::pair<char,char>> _numeric_ranges;
        std::vector<char> _prefixes;
        std::vector<char> _whitespace;
        Trie _operators;

        void __create_token(token_t, uint32, uint32, const char*);
        void __export_token(token_t, const char*, const char*, uint32, uint32);
        void __generate_token(__stream__&);
        
        bool __process_comment(__stream__&);
        bool __process_prefix(__stream__&);
        bool __process_reserved(__stream__& dat);
        bool __process_string(__stream__&);
        bool __process_token(__stream__&);
        bool __process_operators(__stream__&);

    public:

        Lexer(const std::string&);

        bool isPrefix(char c) const;
        bool isReserved(char c) const;
        bool isWhitespace(char c) const;
        bool isAlphaNumeric(char c) const;
        bool isAlpha(char c) const;
        bool isNumeric(char c) const;

        void addAlphaRange(char start, char end);
        void addNumericRange(char start, char end);
        void addAlpha(char elem);
        void addNumeric(char elem);
        void addOperator(const std::string&);
        void addPrefix(char prefix);
        void addWhitespace(char ws);

        bool generate();

        bool good() const;
        uint32 tokenCount() const;

        const TokenHeader& header(uint32) const;
        const char* data(uint32) const;
        uint32 line(uint32) const;
        uint32 column(uint32) const;
    };

}

#endif//__LOOM_PARSER_HPP__