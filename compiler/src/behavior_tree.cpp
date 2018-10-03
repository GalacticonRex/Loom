/*#include "behavior_tree.hpp"
#include <sstream>
#include <iostream>

#define __DEBUG__

namespace Loom {
void Graph::Node::FindTree(Node* root, std::unordered_set<Node*>& found) {
    if ( found.find(root) != found.end() )
        return;

    found.insert(root);

    for ( uint32 i=0;i<root -> _children.size();i++ )
        FindTree(root -> _children[i], found);
}
static bool nodeMatchesToken(Graph::Node* n, LexerStream& toks) {
    return

    // if the token type is none, always return a success 
    n -> tokenType() == Lexer::token_t::NONE || (
        // if there is a token
        toks.good() && 

        // if the token type matches the node type
        toks.front().tokt == n -> tokenType() && (
            // if the node string is empty OR
            n -> matchString().empty() || 

            // the token string matches the node string
            toks.frontData() == n -> matchString() 
        )
    );
}
static bool nodeOperationSucceeds(Graph::Node* n, LexerStream& toks, Graph::State& state) {
    return n -> doPush(toks, state);
}

static bool nodeTypeIsNone(Graph::Node* n) {
    return ( n -> tokenType() == Lexer::token_t::NONE );
}

static void generateError(Graph::Node* n, LexerStream& toks, Graph::State& st, uint32 depth) {
    Graph::State::Error e = {
        toks.front().line, 
        toks.front().col,
        ""
        };

    if ( e < st.error )
        return;

    std::stringstream ss;
    ss << "[From '" << n -> name() << "' @ depth " << depth << "] Got " << Lexer::token(toks.front().tokt);
    if ( *toks.frontData() != '\0' ) {
        ss << " '" << toks.frontData() << "' expected " << Lexer::token(n -> tokenType());
        if ( !n -> matchString().empty() )
            ss << " '" << n -> matchString() << "'";
    } else {
        ss << " expected " << Lexer::token(n -> tokenType());
        if ( !n -> matchString().empty() )
            ss << " '" << n -> matchString() << "'";
    }
    e.msg = ss.str();
    st.error = e;
}

Graph::Result Graph::Node::Traverse(Node* root, LexerStream& toks, State& state) {

    if ( !toks.good() ) {
        #ifdef __DEBUG__
        std::cerr << "traveral is already complete" << std::endl;
        #endif
        return COMPLETE;
    }

    std::vector<Element> list;

    list.push_back({root, toks.index(), 0, (uint32) -1});

    bool found_path = false;
    for(;;) {
        Element& e = list.back();

        #ifdef __DEBUG__
        std::cerr << std::endl << "iteration >>> (" << toks.front().line << ":" << toks.front().col << ") (" << toks.good() << ":" << Lexer::token(toks.front().tokt) << ")";
        uint32 index = -1;
        for ( int32 i=0;i<(int32)list.size()-1;i++ ) {
            if ( index != list[i].index ) {
                std::cerr << std::endl;

                const Lexer::TokenHeader& h = toks.getInternal().header(list[i].index);

                std::cerr << Lexer::token(h.tokt) << " || ";

                index = list[i].index;
            }

            std::cerr << list[i].node -> name() << (( list[i].node -> isAND() ) ? " AND" : " OR") << " (" << list[i].child << ") -> ";
        }
        std::cerr << std::endl << "CURRENT :: " << list.back().node -> name() << std::endl;
        #endif

        bool success = false;
        if ( nodeMatchesToken(e.node, toks) ) {
            if ( nodeOperationSucceeds(e.node, toks, state) ) {
                if ( !nodeTypeIsNone(e.node) )
                    toks.next();

                uint32 local_index = toks.index();

                if ( !e.node -> hasChild(e.child) ) {

                    int32 index = list.size() - 1;
                    for ( ; index >= 0 && 
                            (!list[index].node -> isAND() ||
                             !list[index].node -> hasChild(list[index].child) )
                          ; index-- ) {
                        list[index].node -> doPop(toks, state);
                    }

                    if ( index < 0 ) {
                        found_path = true;
                        break;
                    } else {
                        list.push_back({
                            list[index].node -> child(list[index].child ++ ),
                            local_index,
                            0u,
                            (uint32) index
                        });
                    }
                } else {

                    Element new_e = {
                        e.node -> child(e.child),
                        local_index,
                        0u,
                        (uint32)list.size() - 1
                    };
                    e.child ++ ;
                    list.push_back(new_e);
                }

                success = true;
                state.graph_errors.clear();
            }
        }

        if ( !success ) {

            #ifdef __DEBUG__
            std::cerr << " --- failed" << std::endl;
            #endif

            generateError(list.back().node, toks, state, list.size());

            do {

                if ( !(list.back().node -> error().empty()) )
                    state.graph_errors.push_back({
                        toks.front().line, 
                        toks.front().col, 
                        list.back().node -> error()
                        });

                list.back().node -> doFailure(toks, state);
                if ( list.back().pred != (uint32)-1 && list[list.back().pred].node -> isAND())
                    list[list.back().pred].child -- ;
                list.pop_back();

            } while ( 
                !list.empty() && (
                    list.back().node -> isAND() || 
                    !list.back().node -> hasChild(list.back().child) 
                    ) 
                );
            
            if ( list.empty() )
                break;
            else {
                uint32 local_index = list.back().index;
                if ( !nodeTypeIsNone(list.back().node) )
                    local_index ++ ;
                toks.revert(local_index);

                uint32 index = list.back().child;
                ++ list.back().child;
                list.push_back({
                    list.back().node -> child(index),
                    local_index,
                    0u,
                    (uint32)list.size() - 1
                });
            }
        }

    }

    return (found_path) ? READY : FAILURE;
}

Graph::Node::Node(const std::string& name) :
    _name(name),
    _type(OR),
    _tokt(Lexer::token_t::NONE) { ; }

void Graph::Node::setType(Type t) {
    this -> _type = t;
}
void Graph::Node::setErrorFunction(NodeFunc f) {
    this -> _failure = f;
}
void Graph::Node::setPushFunction(NodeFunc f) {
    this -> _push = f;
}
void Graph::Node::setPopFunction(NodeFunc f) {
    this -> _pop = f;
}
void Graph::Node::setTokenType(Lexer::token_t tokt) {
    this -> _tokt = tokt;
}
void Graph::Node::setTokenString(const std::string& mat) {
    this -> _match = mat;
}
void Graph::Node::setErrorMessage(const std::string& err) {
    this -> _error = err;
}

const std::string& Graph::Node::name() const {
    return this -> _name;
}
const std::string& Graph::Node::error() const {
    return this -> _error;
}
Graph::Type Graph::Node::type() const {
    return this -> _type;
}
const std::string& Graph::Node::matchString() const {
    return this -> _match;
}
Lexer::token_t Graph::Node::tokenType() const {
    return this -> _tokt;
}
bool Graph::Node::isAND() const {
    return this -> _type == AND;
}
bool Graph::Node::isOR() const {
    return this -> _type == OR;
}
void Graph::Node::addChild(Node* n) {
    this -> _children.push_back(n);
}

uint32 Graph::Node::childCount() const {
    return this -> _children.size();
}
bool Graph::Node::hasChild(uint32 i) const {
    return i < this -> _children.size();
}
Graph::Node* Graph::Node::child(uint32 i) const {
    return this -> _children[i];
}
bool Graph::Node::doPush(LexerStream& toks, State& st) const {
    return !_push || _push(toks, st);
}
bool Graph::Node::doPop(LexerStream& toks, State& st) const {
    return !_pop || _pop(toks, st);
}
void Graph::Node::doFailure(LexerStream& toks, State& state) const {
    if ( this -> _failure )
        this -> _failure(toks, state);
}

Graph::Graph() :
    _root(new Node("root")) { ; }

Graph::~Graph() {
    std::unordered_set<Node*> found;
    Node::FindTree(_root, found);
    std::unordered_set<Node*>::iterator iter = found.begin();
    for ( ;iter!=found.end();++iter ) {
        delete *iter;
    }
}

Graph::Node* Graph::root() const {
    return _root;
}

Graph::State::State() :
    error({0,0,""}) { ; }
void Graph::State::clear() {
    this -> identifier.clear();
    this -> error = {0,0,""};
    this -> graph_errors.clear();
}

bool Graph::State::Error::operator<(const Error& e) const {
    return ( line < e.line || (line == e.line && col < e.col ) );
}
bool Graph::State::Error::operator<=(const Error& e) const {
    return ( line < e.line || (line == e.line && col <= e.col ) );
}
bool Graph::State::Error::operator>(const Error& e) const {
    return ( line > e.line || (line == e.line && col > e.col ) );
}
bool Graph::State::Error::operator>=(const Error& e) const {
    return ( line > e.line || (line == e.line && col >= e.col ) );
}

}*/