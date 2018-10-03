#include "assembler.hpp"
#include "fileio.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if ( argc < 2 ) {
        std::fprintf(stderr, "Please provide a valid YARN file to build\n");
        std::exit(-1);
    }

    std::printf("Loading YARN file %s\n", argv[1]);
    std::string data = Loom::load_file(argv[1]);

    try {

        Loom::Object asmbl(data);
        asmbl.assemble();
        asmbl.link();

        const std::vector<uint8_t>& program = asmbl.getProgram();

        const char* save_fname = (argc>=3) ? argv[2] : "output.textile";
        std::printf("Writing TEXTILE file '%s'\n", save_fname);
        Loom::CODE::HEADER& hd = * (Loom::CODE::HEADER*) &program[0];
        Loom::save_file(save_fname, hd.PROG_SIZE, &program[0]);

    } catch ( std::string error ) {
        std::fprintf(stderr, "%s\n", error.c_str());
        std::exit(-1);
    }

    std::exit(0);
}