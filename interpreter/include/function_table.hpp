#ifndef __LOOM_FUNCTION_TABLE_HPP__
#define __LOOM_FUNCTION_TABLE_HPP__

#include "code.hpp"
#include <unordered_map>
#include <string>
#include <vector>

namespace Loom {

    //======================================================================
    //
    // CLASS : FunctionTable
    // The FunctionTable holds a set of functions, which can be localized
    // based on the requirements of a specified program.
    //
    //======================================================================

    struct FunctionTable {

        // the localized table is represented by a single pointer,
        // that points to the head of an array of function pointers
        typedef ASM_FUNC* Local;

    private:

        // unordered map of function names to function pointers
        std::unordered_map<std::string, ASM_FUNC> _ftable;

        // list of tables that have been localized since localized
        // tables aren't freed by the caller, the FunctionTable 
        // deletes them when the FunctionTable itself is deleted
        std::vector<Local> _localized_tables;

    public:

        // when destroyed, the FunctionTable also frees all
        // derived local tables
        ~FunctionTable();

        // adds a new function name/pointer pair to _ftable
        void assignFunction(const std::string& ss, ASM_FUNC a);

        // creates a local table (a list of function pointers) 
        // based on the program argument
        Local localizeTableAgainstProgram(Code*);

    };

}

#endif//__LOOM_FUNCTION_TABLE_HPP__