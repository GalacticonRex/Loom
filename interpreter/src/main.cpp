#include "arguments.hpp"
#include "interpreter.hpp"
#include "function_table.hpp"
#include "fileio.hpp"
#include "sysc.hpp"
#include "device_compiler.hpp"
#include <chrono>

int main(int argc, char** argv) {
    if ( argc == 0 ) {
        std::fprintf(stderr, "Please provide a valid TEXTILE file to run\n");
        std::exit(-1);
    }

    #ifdef _TIME_PROGRAM_
    auto start = std::chrono::high_resolution_clock::now();
    std::printf("-----\nTiming program...\n-----\n\n");
    #endif

    try {

        std::string code_str = Loom::load_file(argv[1]);
        Loom::Code* code = (Loom::Code*) code_str.c_str();

        Loom::FunctionTable table;

        table.assignFunction( "exit",   Loom::System::exit   );
        table.assignFunction( "printf", Loom::System::printf );
        table.assignFunction( "strtol", Loom::System::strtol );

        Loom::CODE::HEADER& hd = * (Loom::CODE::HEADER*) code;
        Loom::Unit* kernels = (Loom::Unit*) (code + hd.KERNEL_OFFSET);

        cl_platform_id platform;
        cl_device_id dev;
        if ( clGetPlatformIDs(1, &platform, NULL) < 0 )
            std::exit(-1);

        if ( clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL) == CL_DEVICE_NOT_FOUND )
            if ( clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL) < 0 )
                std::exit(-1);

        Loom::DeviceCompiler devcmp(dev);
        devcmp.addKernel(kernels[0], code, kernels[1] - kernels[0]);

        //Loom::ThreadPool pool(1, 65536, 1024);
        //Loom::ThreadPool pool(std::thread::hardware_concurrency(), 65536, 1024);
        //Loom::Interpreter::runProgram(pool, table, code, -- argc, ++ argv);

    } catch ( std::string error ) {
        std::fprintf(stderr, "%s\n", error.c_str());
    }

    #ifdef _TIME_PROGRAM_
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float64> dur = end - start;
    std::printf("\n-----\nExecution took %f seconds\n-----\n", dur.count());
    #endif
}
