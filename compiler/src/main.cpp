//#define _DEBUG_

#include "lexer_stream.hpp"
#include "verify_syntax.hpp"
#include "fileio.hpp"
#include <iostream>
#include <cstring>
#include <stack>

using namespace Loom;

static void configureLexer(Lexer&);

int main(int argc, char** argv) {
    if ( argc != 2 ) {
        std::fprintf(stderr, "Please provide a valid LOOM file to build\n");
        std::exit(-1);
    }

    std::printf("Loading LOOM file %s\n", argv[1]);
    std::string loom = load_file(argv[1]);

    try {

        Lexer lex(loom);
        LexerStream stream(lex);

        configureLexer(lex);

        Syntax verify(stream);
        while( verify.statement() );

    } catch( std::string error ) {
        std::cerr << error << std::endl;
    }

    return 0;
}

void configureLexer(Lexer& lex) {
    lex.addAlphaRange('A', 'Z');
    lex.addAlphaRange('a', 'z');
    lex.addAlpha('_');
    lex.addNumericRange('0', '9');
    lex.addWhitespace(' ');
    lex.addWhitespace('\t');
    lex.addWhitespace('\n');
    lex.addPrefix('@');
    lex.addOperator("*");
    lex.addOperator(":");
    lex.addOperator("=");
    lex.addOperator("|");
    lex.addOperator(",");
    lex.addOperator(".");
    lex.addOperator("->");
    lex.addOperator("<-");
    lex.addOperator("?");
    lex.addOperator(";");
}
