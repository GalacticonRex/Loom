#include "sysc.hpp"
#include "code.hpp"
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace Loom {
namespace System {

    void exit(ubyte* page) {
        std::exit(* (Unit*) page);
    }
    void printf(ubyte* page) {
        std::stringstream output;

        const char* cstr = * (const char**) page;

        page += sizeof(const char*);
        for ( ;*cstr!='\0';++cstr ) {
            if ( *cstr == '%' ) {
                ++ cstr;
                switch (*cstr) {
                    case '%': 
                        output << '%';
                        break;
                    case 'd': 
                        output << (*(Unit*)page);
                        page += sizeof(Unit);
                        break;
                    case 's': 
                        output << (*(const char**)page);
                        page += sizeof(const char*);
                        break;
                    case 'p': 
                        output << (*(void**)page);
                        page += sizeof(void*);
                        break;
                }
            } else
                output << *cstr;
        }

        std::printf("%s", output.str().c_str());
        std::fflush(stdout);
    }
    void strtol(ubyte* page) {
        Unit* units = (Unit*) page;
        const char* input = (const char*) units[0];
        units[1] = std::strtol(input, nullptr, 10);
    }
    void print_page(ubyte* page) {
        ubyte* p = * (ubyte**) page;
        if ( p == nullptr ) {
            std::cerr << "ERROR in print_page : Cannot print a null page" << std::endl;
            return;
        }
        std::cerr << " === Address = " << (void*) p << " === ";
        for ( uint32 i=0;i<256;i++ ) {
            if ( i % 8 == 0 )
                std::cerr << std::endl << std::dec << std::setw(4) << i << " | ";
            std::cerr << "[" << std::hex << std::setw(2) << (uint32) p[i] << "]";
        }   std::cerr << std::dec << std::endl;
    }


    // deprecated
    void print_int(ubyte* page) {
        Unit* i = (Unit*) page;
        std::printf("%ld", i[0]);
    }
    void print_hex(ubyte* page) {
        Unit* i = (Unit*) page;
        std::printf("%lx", i[0]);
    }
    void print_string(ubyte* page) {
        const char* i = (const char*) (* (Unit*) page);
        std::printf("%s", i);
    }
    void print_bool(ubyte* page) {
        Unit* i = (Unit*) page;
        std::printf("%s", i[0] ? "True" : "False");
    }
    void sqrt(ubyte* page) {
        Unit* units = (Unit*) page;
        Unit& output = * (Unit*) units[0];
        output = std::sqrt(output);
    }

    bool isprime(Unit value) {
        Unit sq = std::sqrt(value);
        for ( Unit i=2;i<=sq;i++ ) {
            if ( value % i == 0 ) {
                return false;
            }
        }
        return true;
    }

    void test_primes(ubyte* page) {
        Unit* upage = (Unit*) page;
        Unit* value = * (Unit**) page;
        Unit start = upage[1];
        Unit end = upage[2];

        for ( Unit i=start;i<end;i++ ) {
            if ( isprime(i) ) {
                (*value) ++ ;
            }
        }
    }

} }