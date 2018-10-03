#ifndef __DEVICE_COMPILER_HPP__
#define __DEVICE_COMPILER_HPP__

#include "code.hpp"
#include "types.hpp"
#include "cl/opencl.h"
#include <unordered_map>

namespace Loom {

    // static class DeviceCompiler
    // Accepts Yarn bytecode and compiles this into a source-code
    // that an external device can understand.
    struct DeviceCompiler {
    private:
        
        cl_device_id _cl_dev;
        cl_context _cl_ctx;
        std::unordered_map<Unit, cl_kernel> _cl_programs;

    public:

        DeviceCompiler(cl_device_id device);
        void addKernel(Unit location_in_program, ubyte* bytecode, Unit length);

    };

};

#endif//__DEVICE_COMPILER_HPP__
