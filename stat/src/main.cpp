#include "fileio.hpp"
#include "types.hpp"
#include <iostream>
#include <getopt.h>

int main(int argc, char** argv) {
    if ( argc < 2 ) {
        std::fprintf(stderr, "Please provide a valid TEXTILE file to stat\n");
        std::exit(-1);
    }

    bool display_info = false,
         display_string = false,
         display_header = false,
         display_text = false,
         display_func = false,
         display_kernel = false,
         display_raw = false;

    int opt;
    while ((opt = getopt(argc, argv, "iestfkr")) != -1) {
        switch (opt) {
            case 'i':
                display_info = true;
                break;
            case 'e':
                display_header = true;
                break;
            case 's':
                display_string = true;
                break;
            case 't':
                display_text = true;
                break;
            case 'f':
                display_func = true;
                break;
            case 'k':
                display_kernel = true;
                break;
            case 'r':
                display_raw = true;
                break;
            default:
                fprintf(stderr, "Usage: %s [-iestfkr] file\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if ( !display_info && 
         !display_string && 
         !display_header && 
         !display_text && 
         !display_func && 
         !display_kernel && 
         !display_raw )
        display_info = true;

    for (int index = optind; index < argc; index++) {

        std::string textile = Loom::load_file(argv[index]);

        try {

            const char* raw_textile = textile.c_str();
            Loom::CODE::HEADER& hd = * (Loom::CODE::HEADER*) raw_textile;

            std::cout << " ====== " << argv[index] << " | Loom v" << hd.VERSION[0] << "." << hd.VERSION[1] << "." << hd.VERSION[2] << " ====== " << std::endl << std::endl;

            if ( display_info ) {
                std::cout << " === Info === " << std::endl;
                std::cout << std::endl;
                std::cout << "     Program Size:       " << hd.PROG_SIZE << std::endl;
                std::cout << std::endl;
                std::cout << "     System Call Offset: " << hd.CALL_TABLE_OFFSET << std::endl;
                std::cout << "     System Call Size:   " << hd.CALL_TABLE_SIZE << std::endl;
                std::cout << std::endl;
                std::cout << "     Text Offset:        " << hd.TEXT_OFFSET << std::endl;
                std::cout << "     Text Size:          " << hd.TEXT_SIZE << std::endl;
                std::cout << std::endl;
                std::cout << "     Data Offset:        " << hd.DATA_OFFSET << std::endl;
                std::cout << "     Data Size:          " << hd.DATA_SIZE << std::endl;
                std::cout << std::endl;
            }

            if ( display_header ) {
                std::cout << " === Extra Info === " << std::endl;
                std::cout << std::endl;
                std::cout << "     Entry Point:        " << hd.ENTRY_POINT << std::endl;
                std::cout << "     Parses Arguments:   " << ( (hd.FLAGS&Loom::CODE::HEADER::FLAG_PARSE_ARGS) ? "TRUE" : "FALSE" ) << std::endl;
                std::cout << std::endl;
            }

            if ( display_func ) {
                std::cout << " === Required Functions === " << std::endl;
                std::cout << std::endl;
                const char* syscl = raw_textile + hd.CALL_TABLE_OFFSET;
                while ( *syscl != '\0' ) {
                    std::string name((const char*) syscl);
                    std::cout << "   - '" << name << "'" << std::endl;
                    syscl += name.size() + 1;
                }
                std::cout << std::endl;
            }

            if ( display_string ) {
                std::cout << " === Strings === " << std::endl;
                std::cout << std::endl;
                const char* data = raw_textile + hd.DATA_OFFSET;
                while ( *data != '\0' ) {
                    std::string name(data);
                    std::cout << "   - '" << name << "'" << std::endl;
                    data += name.size() + 1;
                }
                std::cout << std::endl;
            }

            if ( display_text ) {
                std::cout << " === Text === " << std::endl << std::endl;
                for ( Loom::Unit i = 0;i<hd.TEXT_SIZE; ) {
                    printf("%8lu | ", i+hd.TEXT_OFFSET);
                    for ( Loom::Unit j=0;j<16 && i<hd.TEXT_SIZE; j++,i++ ) {
                        printf("%02x ", (ubyte) raw_textile[i+hd.TEXT_OFFSET]);
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;
            }

            if ( display_kernel ) {
                std::cout << " === Kernel === " << std::endl;
                for ( Loom::Unit i = 0;i<hd.KERNEL_SIZE;i+=2*sizeof(Loom::Unit) ) {
                    Loom::Unit start = * ((Loom::Unit*)(raw_textile + hd.KERNEL_OFFSET + i));
                    Loom::Unit end   = * ((Loom::Unit*)(raw_textile + hd.KERNEL_OFFSET + i)+1);
                    std::printf("\nKernel %lu (%lu - %lu):\n", i/2/sizeof(Loom::Unit)+1, start, end);
                    for (Loom::Unit j=start;j<end;j++ ) {
                        if ( ((j-start) % 16) == 0 ) std::printf("\n");
                        std::printf("%02x ", (ubyte) raw_textile[j]);
                    }
                    std::printf("\n");
                }
                std::cout << std::endl;
            }

            if ( display_raw ) {
                std::cout << " === Raw === " << std::endl << std::endl;
                for ( Loom::Unit i = 0;i<hd.PROG_SIZE; ) {
                    printf("%8lu | ", i);
                    for ( Loom::Unit j=0;j<16 && i<hd.PROG_SIZE; j++,i++ ) {
                        printf("%02x ", (ubyte) raw_textile[i]);
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;
            }
            std::cout << " ===================== " << std::endl;

        } catch ( std::string error ) {
            std::fprintf(stderr, "%s\n", error.c_str());
        }

    }

    std::exit(0);
}