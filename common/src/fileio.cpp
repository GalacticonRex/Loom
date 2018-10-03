#include "fileio.hpp"
#include <fstream>

namespace Loom {

std::string load_file(const char* fname) {
    std::ifstream t(fname);
    if (!t.good()) {
        std::fprintf(stderr, "Invalid filename %s\n", fname);
        std::exit(-1);
    }
    std::string str;

    t.seekg(0, std::ios::end);   
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
                std::istreambuf_iterator<char>());

    return str;
}

void save_file(const char* fname, uint32 len, const ubyte* data) {
    std::ofstream t(fname, std::ios::binary |
                           std::ios::out);
    t.write((const char*)data, len);
    t.close();
}

}