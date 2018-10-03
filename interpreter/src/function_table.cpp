#include "arguments.hpp"
#include "function_table.hpp"
#include "types.hpp"
#include "util.hpp"
#include <cstring>

namespace Loom {

    FunctionTable::~FunctionTable() {
        for ( uint32 i=0;i<_localized_tables.size();i++ ) {
            delete[] _localized_tables[i];
        }
    }

    void FunctionTable::assignFunction(const std::string& func_string, ASM_FUNC func) {
        std::unordered_map<std::string, ASM_FUNC>::const_iterator i =
            _ftable.find( func_string );
        if ( i != _ftable.end() )
            sendError("Duplicate function name %s in function table", func_string.c_str());
        _ftable.insert(i, std::make_pair(func_string, func));
    }

    FunctionTable::Local FunctionTable::localizeTableAgainstProgram(Code* code) {
        #ifdef _DEBUG_
        fprintf(stderr, "Localizing Function Table\n");
        #endif//_DEBUG_

        CODE::HEADER h = * (CODE::HEADER*) code;
        std::vector<ASM_FUNC> buffer;
        
        Code* syscl = code + h.CALL_TABLE_OFFSET;
        while ( *syscl != '\0' ) {
            std::string name((const char*)syscl);
            std::unordered_map<std::string, ASM_FUNC>::const_iterator iter =
                _ftable.find( name );

            if ( iter == _ftable.end() )
                sendError("Program requires unavailable function '%s'", name.c_str());

            #ifdef _DEBUG_
            fprintf(stderr, "    syscall[%lu] = %s (%p)\n", buffer.size(), name.c_str(), ((void*)iter -> second));
            #endif//_DEBUG_
            
            buffer.push_back(iter -> second);
            syscl += name.size() + 1;
        }

        Local result = new ASM_FUNC[buffer.size()];
        for ( uint32 i=0;i<buffer.size();i++ )
            result[i] = buffer[i];

        _localized_tables.push_back(result);

        return result;
    }

}