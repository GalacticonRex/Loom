#ifndef __LOOM_TRIE_HPP__
#define __LOOM_TRIE_HPP__

#include "types.hpp"
#include <vector>
#include <stack>
#include <unordered_map>

namespace Loom {

    struct Trie {
    private:
        
        struct __entry__ {
            typedef std::unordered_map<char, __entry__*> M;
            char local;
            bool is_string;
            uint32 branches;
            M children;

            __entry__(char c, bool b) :
                local(c), is_string(b), branches(0) { ; }
        };

        uint32 __test(const __entry__*, const char*, uint32) const;
        bool __valid(const __entry__*, const char*) const;
        bool __insert(__entry__*, const char*);
        bool __remove(__entry__*, const char*);

        __entry__ _root;
        uint32 _size;

    public:

        struct iterator {
        private:
            struct __stack_value__ {
                const __entry__* _entry;
                std::unordered_map<char, __entry__*>::const_iterator _iter;

                __stack_value__(const __entry__*);
            };
            std::string _word;
            std::stack<__stack_value__> _stack;

            bool __iterate_once();

        public:
            iterator(const __entry__* value);
            bool next();
            const std::string& get() const;
        };

        Trie();

        iterator begin() const;

        uint32 size() const;

        uint32 match(const char*) const;
        bool valid(const char*) const;
        bool insert(const char*);
        bool remove(const char*);

    };

}

#endif//__LOOM_UTIL_HPP__