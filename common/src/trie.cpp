#include "trie.hpp"
#include <iostream>

namespace Loom {

    Trie::iterator::__stack_value__::__stack_value__(const __entry__* e) :
        _entry(e), _iter(e -> children.begin()) { ; }

    bool Trie::iterator::__iterate_once() {
        if ( _stack.empty() )
            return false;

        __stack_value__& s = _stack.top();
        if ( s._iter == s._entry -> children.end() ) {
            if ( !_word.empty() )
                _word.pop_back();
            _stack.pop();
            if ( _stack.empty() )
                return false;
        } else {
            if ( s._iter -> first != '\0' )
                _word += s._iter -> first;
            _stack.push(__stack_value__(s._iter -> second));
            s._iter ++ ;
        }
        return true;
    }

    Trie::iterator::iterator(const __entry__* value) {
        _stack.push(__stack_value__(value));
        (void) next();
    }
    bool Trie::iterator::next() {
        do {
            if ( !__iterate_once() )
                return false;
        } while ( !(_stack.top()._entry -> is_string) );
        return true;
    }
    const std::string& Trie::iterator::get() const {
        return _word;
    }

    Trie::Trie() : _root('\0', false) { ; }

    Trie::iterator Trie::begin() const {
        return iterator(&_root);
    }

    uint32 Trie::size() const {
        return _size;
    }

    uint32 Trie::match(const char* str) const {
        return __test(&_root, str, 0);
    }
    bool Trie::valid(const char* str) const {
        return __valid(&_root, str);
    }
    bool Trie::insert(const char* str) {
        return __insert(&_root, str);
    }

    uint32 Trie::__test(const __entry__* par, const char* str, uint32 value) const {
        if ( *str == '\0' )
            return (par -> is_string) ? value : 0;

        __entry__::M::const_iterator i = par -> children.find(*str);
        
        uint32 ret;

        if ( i == par -> children.end() )
            ret = 0;
        else
            ret = __test(i -> second, str+1, value + 1);
        
        if ( ret == 0 )
            return (par -> is_string) ? value : 0;
        else
            return ret;
    }
    bool Trie::__valid(const __entry__* par, const char* str) const {
        if ( *str == '\0' ) 
            return par -> is_string;

        __entry__::M::const_iterator i = par -> children.find(*str);
        if ( i == par -> children . end() )
            return false;
        else
            return __valid(i -> second, str+1);
    }
    bool Trie::__insert(__entry__* par, const char* str) {
        if ( *str == '\0' ) {
            bool prev = par -> is_string;
            par -> is_string = true;
            return prev;
        }

        par -> branches ++ ;

        __entry__::M::iterator i = par -> children.find(*str);
        __entry__* e;
        if ( i == par -> children . end() ) {
            e = new __entry__(*str, false);
            par -> children.insert(std::make_pair(*str, e));
        } else {
            e = i -> second;
        }

        return __insert(e, str + 1);
    }
    /*bool Trie::__remove(__entry__* par, const char* str) {
        if ( *str == '\0' ) {
            if ( par -> in_string ) {
                par -> in_string = false;
                return true;
            } else
                return false;
        }

        __entry__::M::iterator i = par -> find(*str);
        if ( i == par -> end() )
            return false;

        if ( __remove(i -> second, str+1) ) {

            return true;
        } else {
            return false;
        }
    }*/

}