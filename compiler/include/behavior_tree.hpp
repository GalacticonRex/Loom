#ifndef __LOOM_BEHAVIOUR_TREE_HPP__
#define __LOOM_BEHAVIOUR_TREE_HPP__

#include <vector>
#include <stack>
#include <functional>
#include <unordered_set>

#include "lexer_stream.hpp"

namespace Loom {

    struct Graph {

        enum Type { OR, AND };
        enum Result { READY, FAILURE, COMPLETE };

        struct State {
            struct Error {
                uint32 line;
                uint32 col;
                std::string msg;

                bool operator<(const Error&) const;
                bool operator<=(const Error&) const;
                bool operator>(const Error&) const;
                bool operator>=(const Error&) const;
            };

        // immediate
            std::vector<std::string> identifier;
            Error error;
            std::vector<Error> graph_errors;
            
        // accumulated
            std::vector<std::string> textile;

        // methods
            State();
            void clear();
        };

        typedef std::function<bool(LexerStream&, State&)> NodeFunc;

        struct Node {

            struct Element {
                Node* node;
                uint32 index;
                uint32 child;
                uint32 pred;
            };

        private:

            std::string _name;

            Type _type;

            std::vector<Node*> _children;

            NodeFunc _push;
            NodeFunc _pop;
            Lexer::token_t _tokt;
            std::string _match;

            NodeFunc _failure;
            std::string _error;

        public:
            Node(const std::string&);

            void setType(Type);
            void setPushFunction(NodeFunc);
            void setPopFunction(NodeFunc);
            void setErrorFunction(NodeFunc);
            void setTokenType(Lexer::token_t);
            void setTokenString(const std::string&);
            void setErrorMessage(const std::string&);
            void addChild(Node*);

            const std::string& name() const;
            const std::string& error() const;
            Type type() const;
            const std::string& matchString() const;
            Lexer::token_t tokenType() const;
            bool isAND() const;
            bool isOR() const;
            uint32 childCount() const;
            bool hasChild(uint32) const;
            Node* child(uint32) const;
            bool doPush(LexerStream&, State&) const;
            bool doPop(LexerStream&, State&) const;
            void doFailure(LexerStream&, State&) const;

            static void FindTree(Node*, std::unordered_set<Node*>&);
            static Result Traverse(Node*, LexerStream&, State&);
        };

    private:
        Node* _root;

    public:
        Graph();
        ~Graph();

        Node* root() const;
    };

}

#endif//__LOOM_BEHAVIOUR_TREE_HPP__