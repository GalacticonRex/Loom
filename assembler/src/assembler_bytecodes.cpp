#include "assembler.hpp"
#include "assembler_helpers.hpp"

namespace Loom {

void Object::__noop() {
    __text_code(CODE::NOOP);
}
void Object::__debug() {
    __text_code(CODE::DEBUG);
    __argument("DEBUG", 1, ASM_TYPE_DATA);
}

void Object::__assg() {
    __text_code(CODE::ASSG);
    __argument("ASSG", 1, ASM_TYPE_ANY);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("ASSG", 2, ASM_TYPE_VARIABLE);
}
void Object::__sysc() {
    __text_code(CODE::SYSC);
    
    need(Lexer::token_t::STRING, _stream);
    std::string reference = _stream.frontData();

    __references_t__::const_iterator match =
        _system_calls.find(reference);
    if ( match == _references.end() ) {
        fprintf(stderr, "Adding function requirement '%s'\n", _stream.frontData());
        _system_calls.insert(match, std::make_pair(reference, _func_count));
        __text_unit(_func_count);
        ++ _func_count;
        for (const char* i = _stream.frontData();*i!='\0';++i)
            _func.push_back(*i);
        _func.push_back('\0');
    } else {
        __text_unit(match -> second);
    }

    _stream.next();
}
void Object::__call() {
    __text_code(CODE::CALL);
    __argument("CALL", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
}
void Object::__retn() {
    __text_code(CODE::RETN);
}
void Object::__push() {
    __text_code(CODE::PUSH);
    __argument("PUSH", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL);
}
void Object::__pop() {
    __text_code(CODE::POP);
}
void Object::__make() {
    __text_code(CODE::MAKE);
    __argument("MAKE", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB );
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("MAKE", 2, ASM_TYPE_VARIABLE);
}
void Object::__free() {
    __text_code(CODE::FREE);
    __argument("FREE", 1, ASM_TYPE_VARIABLE);
}
void Object::__jump() {
    __text_code(CODE::JUMP);
    __argument("JUMP", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
}
void Object::__jeq() {
    __text_code(CODE::JEQ);
    __argument("JEQ", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
    __argument("JEQ", 2, ASM_TYPE_ANY);
    __argument("JEQ", 3, ASM_TYPE_ANY);
}
void Object::__jneq() {
    __text_code(CODE::JNEQ);
    __argument("JNEQ", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
    __argument("JNEQ", 2, ASM_TYPE_ANY);
    __argument("JNEQ", 3, ASM_TYPE_ANY);
}
void Object::__jls() {
    __text_code(CODE::JLS);
    __argument("JLS", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
    __argument("JLS", 2, ASM_TYPE_ANY);
    __argument("JLS", 3, ASM_TYPE_ANY);
}
void Object::__jlse() {
    __text_code(CODE::JLSE);
    __argument("JLSE", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
    __argument("JLSE", 2, ASM_TYPE_ANY);
    __argument("JLSE", 3, ASM_TYPE_ANY);
}
void Object::__jgr() {
    __text_code(CODE::JGR);
    __argument("JGR", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
    __argument("JGR", 2, ASM_TYPE_ANY);
    __argument("JGR", 3, ASM_TYPE_ANY);
}
void Object::__jgre() {
    __text_code(CODE::JGRE);
    __argument("JGRE", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
    __argument("JGRE", 2, ASM_TYPE_ANY);
    __argument("JGRE", 3, ASM_TYPE_ANY);
}
void Object::__jz() {
    __text_code(CODE::JZ);
    __argument("JZ", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
    __argument("JZ", 2, ASM_TYPE_ANY);
}
void Object::__jnz() {
    __text_code(CODE::JNZ);
    __argument("JNZ", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
    __argument("JNZ", 2, ASM_TYPE_ANY);
}
void Object::__incr() {
    __text_code(CODE::INCR);
    __argument("INCR", 1, ASM_TYPE_VARIABLE);
}
void Object::__decr() {
    __text_code(CODE::DECR);
    __argument("DECR", 1, ASM_TYPE_VARIABLE);
}
void Object::__ineg() {
    __text_code(CODE::INEG);
    __argument("INEG", 1, ASM_TYPE_VARIABLE);
}
void Object::__iadd() {
    __text_code(CODE::IADD);
    __argument("IADD", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    __argument("IADD", 2, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("IADD", 3, ASM_TYPE_VARIABLE);
}
void Object::__isub() {
    __text_code(CODE::ISUB);
    __argument("ISUB", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    __argument("ISUB", 2, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("ISUB", 3, ASM_TYPE_VARIABLE);
}
void Object::__imul() {
    __text_code(CODE::IMUL);
    __argument("IMUL", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    __argument("IMUL", 2, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("IMUL", 3, ASM_TYPE_VARIABLE);
}
void Object::__idiv() {
    __text_code(CODE::IDIV);
    __argument("IDIV", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    __argument("IDIV", 2, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("IDIV", 3, ASM_TYPE_VARIABLE);
}
void Object::__ipow() {
    __text_code(CODE::IPOW);
    __argument("IPOW", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    __argument("IPOW", 2, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("IPOW", 3, ASM_TYPE_VARIABLE);
}
void Object::__imod() {
    __text_code(CODE::IMOD);
    __argument("IMOD", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    __argument("IMOD", 2, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("IMOD", 3, ASM_TYPE_VARIABLE);
}
void Object::__isqrt() {
    __text_code(CODE::ISQRT);
    __argument("ISQRT", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("ISQRT", 2, ASM_TYPE_VARIABLE);
}
void Object::__ilog() {
    
}
void Object::__sin() {
    
}
void Object::__cos() {
    
}
void Object::__tan() {
    
}
void Object::__fneg() {
    
}
void Object::__fadd() {
    
}
void Object::__fsub() {
    
}
void Object::__fmul() {
    
}
void Object::__fdiv() {
    
}
void Object::__fpow() {
    
}
void Object::__fmod() {
    
}
void Object::__fsqrt() {
    
}
void Object::__flog() {
    
}
void Object::__fvneg() {
    
}
void Object::__fvadd() {
    
}
void Object::__fvsub() {
    
}
void Object::__fvmul() {
    
}
void Object::__fvdiv() {
    
}
void Object::__fvdot() {
    
}
void Object::__fvcross() {
    
}
void Object::__mneg() {
    
}
void Object::__madd() {
    
}
void Object::__msub() {
    
}
void Object::__mmul() {
    
}
void Object::__mtrans() {
    
}
void Object::__rpc() {
    __text_code(CODE::RPC);
    __argument("RPC", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
}
void Object::__rpcs() {
    __text_code(CODE::RPCS);
    __argument("RPCS", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
}
void Object::__kernel1() {
    __text_code(CODE::KERN1);
    __argument("KERN1", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
}
void Object::__kernel2() {
    __text_code(CODE::KERN2);
    __argument("KERN2", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
}
void Object::__kernel3() {
    __text_code(CODE::KERN3);
    __argument("KERN3", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
}
void Object::__reduction() {
    __text_code(CODE::REDC);
    __argument("REDC", 1, ASM_TYPE_TEXT | ASM_TYPE_LOCAL);
}
void Object::__scan() {
    
}
void Object::__sort() {
    
}
void Object::__stable_sort() {
    
}
void Object::__start() {
    __text_code(CODE::START);
    __argument("START", 1, ASM_TYPE_VARIABLE);
}
void Object::__stop() {
    __text_code(CODE::STOP);
    __argument("STOP", 1, ASM_TYPE_VARIABLE);
}
void Object::__sync() {
    __text_code(CODE::SYNC);
    __argument("SYNC", 1, ASM_TYPE_VARIABLE);
}
void Object::__config1() {
    __text_code(CODE::CNFG1);
    __argument("CNFG1", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
}
void Object::__config2() {
    __text_code(CODE::CNFG2);
    __argument("CNFG2", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
}
void Object::__config3() {
    __text_code(CODE::CNFG3);
    __argument("CNFG3", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
}
void Object::__copy() {
    __text_code(CODE::COPY);
    __argument("COPY", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
}
void Object::__buffer() {
    __text_code(CODE::BUFFER);
    
    need(Lexer::token_t::TOKEN, _stream);
    std::string access = _stream.frontData();
    if ( access == "READ" )
        __text_code(CODE::CONST_READ);
    else if ( access == "WRITE" )
        __text_code(CODE::CONST_WRITE);
    else if ( access == "READWRITE" )
        __text_code(CODE::CONST_READWRITE);
    else
        sendError("Unknown buffer access type '%s' (%d:%d)",
            access.c_str(),
            _stream.front().line,
            _stream.front().col
            );
    _stream.next();

    __argument("BUFFER", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    __argument("BUFFER", 2, ASM_TYPE_VARIABLE);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("BUFFER", 2, ASM_TYPE_VARIABLE);
}

void Object::__index1() {
    __text_code(CODE::INDEX1);
    __argument("INDEX1", 1, ASM_TYPE_VARIABLE);
}
void Object::__index2() {
    __text_code(CODE::INDEX2);
    __argument("INDEX2", 1, ASM_TYPE_VARIABLE);
}
void Object::__index3() {
    __text_code(CODE::INDEX3);
    __argument("INDEX3", 1, ASM_TYPE_VARIABLE);
}
void Object::__karg() {
    __text_code(CODE::KARG);
    __argument("KARG", 1, ASM_TYPE_VALUE | ASM_TYPE_LOCAL | ASM_TYPE_GLOB);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("KARG", 2, ASM_TYPE_VARIABLE);
}

// File Access Functions
void Object::__file_open(){
    __text_code(CODE::FILEOPEN);
    __argument("FILEOPEN", 1, ASM_TYPE_DATA | ASM_TYPE_VARIABLE);
    __argument("FILEOPEN", 2, ASM_TYPE_DATA | ASM_TYPE_VARIABLE);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("FILEOPEN", 3, ASM_TYPE_VARIABLE);
}
void Object::__file_close(){
    __text_code(CODE::FILECLOSE);
    __argument("FILECLOSE", 1, ASM_TYPE_VARIABLE);
}
void Object::__file_read(){
    __text_code(CODE::FILEREAD);
    __argument("FILEREAD", 1, ASM_TYPE_LITERAL | ASM_TYPE_LOCAL);
    __argument("FILEREAD", 2, ASM_TYPE_LITERAL | ASM_TYPE_LOCAL);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("FILEREAD", 3, ASM_TYPE_VARIABLE);
}
void Object::__file_write(){
    __text_code(CODE::FILEWRITE);
    __argument("FILEWRITE", 1, ASM_TYPE_LITERAL | ASM_TYPE_LOCAL);
    __argument("FILEWRITE", 2, ASM_TYPE_LITERAL | ASM_TYPE_LOCAL);
    need(Lexer::token_t::SYMBOL, ";", _stream);
    _stream.next();
    __argument("FILEWRITE", 3, ASM_TYPE_VARIABLE);
}
void Object::__file_seek(){
    __text_code(CODE::FILESEEK);
}
void Object::__file_tell(){
    __text_code(CODE::FILETELL);
}
void Object::__file_reset(){
    __text_code(CODE::FILERESET);
}

// Generic Functions
void Object::__printf(){
    __text_code(CODE::PRINTF);
}
void Object::__fprintf(){
    __text_code(CODE::FPRINTF);
}
void Object::__memcpy(){
    __text_code(CODE::MEMCPY);
}

}