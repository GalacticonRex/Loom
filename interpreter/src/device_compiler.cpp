#include "util.hpp"
#include "device_compiler.hpp"
#include <vector>
#include <map>

namespace Loom {

    /*
    // Identify a platform
    if(clGetPlatformIDs(1, &platform, NULL) < 0)
        return;

    // Access a device
    if( clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL) == CL_DEVICE_NOT_FOUND || 
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL) < 0 )
        return;

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &error);
    if ( error < 0 )
        return;
    */

DeviceCompiler::DeviceCompiler(cl_device_id device) {
    int error;

    _cl_dev = device;
    _cl_ctx = clCreateContext(NULL, 1, &_cl_dev, NULL, NULL, &error);
    if ( error < 0 )
        sendError("Cannot instantiate compiler on device");
}

static std::string string_lhs(Code type, Unit value) {
    std::string result;
    switch ( type ) {
        case CODE::GLOB: result += "*";
        case CODE::LOCAL: result += "*";
        case CODE::ADDR:
            result += "_" + std::to_string(value);
            break;
        default:
            sendError("Invalid type for LHS");
    }
    return result;
}
static std::string string_rhs(Code type, Unit value) {
    std::string result;
    switch ( type ) {
        case CODE::VALUE:
            result = std::to_string(value);
            break;
        case CODE::GLOB: result += "*";
            goto string_rhs_local;
        case CODE::ADDR: result += "(void*)&";
        case CODE::LOCAL:
            string_rhs_local:
            result += "_" + std::to_string(value);
            break;
        default:
            sendError("Invalid type for RHS");
    }
    return result;
}

void DeviceCompiler::addKernel(Unit location_in_program, ubyte* bytecode, Unit length) {
    std::string head =
        std::string("__kernel void kernel") + std::to_string(location_in_program) + "(";
    std::string decl;
    std::string body;

    std::vector<ubyte*> instructions;

    bytecode = bytecode + location_in_program;
    ubyte* terminate = bytecode + length;
    while ( bytecode < terminate ) {
        
        instructions.push_back(bytecode);
        Code cmd = *bytecode;
        ++ bytecode;

        std::fprintf(stderr, "%s ", CODE::toString(cmd));

        switch ( cmd ) {
            case CODE::RETN:
                break;
            case CODE::INDEX1:
                bytecode += 1 * sizeof(Unit);
                break;
            case CODE::KARG:
                bytecode += 2 * sizeof(Unit);
                break;
            case CODE::IADD:
            case CODE::IMUL:
                bytecode += 3 * sizeof(Unit);
                break;
        }

    }

    std::fprintf(stderr, "\n");

    std::unordered_map<Unit, std::string> var_type;
    std::unordered_map<Unit, std::string>::iterator var_iter;
    std::map<Unit, std::string> arguments;
    std::string binop;
    for ( ubyte* inst : instructions ) {
        ubyte cmd = *inst;
        ++ inst;
        switch (cmd) {
            case CODE::INDEX1: {
                Code type = (Code) * (inst++);
                Unit value = * (Unit*) inst;
                inst += sizeof(Unit);

                var_iter = var_type.find(value);
                if ( var_iter == var_type.end() ) {
                    decl += "long _" + std::to_string(value) + " = 0;\n";
                    var_type.insert(std::make_pair(value, "long"));
                }

                body += string_lhs(type, value) + " = get_local_id(0);\n";
            } break;
            case CODE::KARG: {
                Code type[2];
                Unit value[2];
                for ( uint32 i=0;i<2;i++ ) {
                    type[i] = (Code) * (inst++);
                    value[i] = * (Unit*) inst;
                    inst += sizeof(Unit);
                }

                var_iter = var_type.find(value[1]);
                if ( var_iter == var_type.end() )
                    var_type.insert(std::make_pair(value[1], "long"));

                arguments.insert(std::make_pair(value[0], "long _" + std::to_string(value[1])));
            } break;
            case CODE::IADD: binop = "+";
                goto binary_operator;
            case CODE::IMUL: binop = "*";
                goto binary_operator;
            binary_operator: {
                Code type[3];
                Unit value[3];
                for ( uint32 i=0;i<3;i++ ) {
                    type[i] = (Code) * (inst++);
                    value[i] = * (Unit*) inst;
                    inst += sizeof(Unit);
                }

                var_iter = var_type.find(value[2]);
                if ( var_iter == var_type.end() ) {
                    decl += "long _" + std::to_string(value[2]) + " = 0;\n";
                    var_type.insert(std::make_pair(value[2], "long"));
                }

                body += string_lhs(type[2], value[2]) + " = (" +
                        string_rhs(type[0], value[0]) + ")" + binop + "(" +
                        string_rhs(type[1], value[1]) + ");\n";
            } break;
        }
    }

    for ( std::pair<Unit, std::string> row : arguments )
        head += row.second + ", ";
    if ( !arguments.empty() )
        head = head.substr(0, head.size()-2);

    std::string program = head + ") {\n" + decl + body + "}";

    std::fprintf(stderr, "%s\n", program.c_str());


    /*int error;

    //Create program from file 
    //Creates a program from the source code in the add_numbers.cl file. 
    //Specifically, the code reads the file's content into a char array 
    //called program_buffer, and then calls clCreateProgramWithSource.

    program = clCreateProgramWithSource(_cl_ctx, 1, 
    (const char**)&program_buffer, &program_size, &err);
    if(err < 0) {
    perror("Couldn't create the program");
    exit(1);
    }
    free(program_buffer);

    //Build program 
    //The fourth parameter accepts options that configure the compilation. 
    //These are similar to the flags used by gcc. For example, you can 
    //define a macro with the option -DMACRO=VALUE and turn off optimization 
    //with -cl-opt-disable.
    
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err < 0) {

    // Find size of log and print to std output
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
    0, NULL, &log_size);
    program_log = (char*) malloc(log_size + 1);
    program_log[log_size] = '\0';
    clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
    log_size + 1, program_log, NULL);
    printf("%s\n", program_log);
    free(program_log);
    exit(1);
    }
    */


}

}
