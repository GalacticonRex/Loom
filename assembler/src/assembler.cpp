#include "assembler.hpp"
#include "assembler_helpers.hpp"
#include "util.hpp"
#include "version.hpp"
#include <cstring>
#include <cmath>
#include <fstream>
#include <set>

namespace Loom {

    Object::__conf_lexer__::__conf_lexer__(Lexer& toks) {
        toks.addAlphaRange('A', 'Z');
        toks.addAlphaRange('a', 'z');
        toks.addAlpha('_');
        toks.addNumericRange('0', '9');
        toks.addWhitespace(' ');
        toks.addWhitespace('\t');
        toks.addPrefix('@');
        toks.addOperator(":");
        toks.addOperator(".");
        toks.addOperator("-");
        toks.addOperator("=");
        toks.addOperator(";");
        toks.addOperator("\n");
    }
    Object::Object(const std::string& raw_yarn_data) :
        _lexer(raw_yarn_data+"\n"),
        _lexer_config(_lexer),
        _stream(_lexer),
        _func_count(0),
        _in_kernel_mode(false),
        _meta_entry_point((uint32)-1),
        _meta_parse_args(false) {
        _meta_stack_pointers.push(0);
    }

    // NOT CORRECT
    void Object::include(const std::string& yarn_file_name) {
        for ( uint32 i=0;i<_meta_search.size();i++ ) {
            std::string fname = _meta_search[i] + "/" + yarn_file_name;
            printf("Checking file '%s'\n", fname.c_str());
            std::ifstream incfile(fname);
            if ( incfile.good() ) {

                printf("Including '%s'\n", fname.c_str());

                std::string data;

                incfile.seekg(0, std::ios::end);
                data.reserve(incfile.tellg());
                incfile.seekg(0, std::ios::beg);

                data.assign(std::istreambuf_iterator<char>(incfile),
                            std::istreambuf_iterator<char>());

                Object tmp(data);
                tmp.assemble();

                // insert unlinked references
                {
                    __needs_reference_t__::iterator iter =
                        tmp._needs_reference.begin();
                    for ( ;iter!=tmp._needs_reference.end();++iter )
                        _needs_reference.insert(
                            std::make_pair(
                                iter -> first + _text.size(), 
                                iter -> second
                                )
                            );
                }

                // insert reference locations
                {
                    __references_t__::iterator iter =
                        tmp._references.begin();
                    for ( ;iter!=tmp._references.end();++iter ) {
                        std::string name = iter -> first;
                        Unit loc = iter -> second;

                        __types_t__::iterator iter =
                            tmp._types.begin();
                        if ( iter != tmp._types.end() ) {
                            if ( iter -> second == ASM_TYPE_TEXT) {
                                loc += _text.size();
                            } else if ( iter -> second == ASM_TYPE_DATA ) {
                                loc += _data.size();loc += _data.size();
                            }
                        }
                        _references.insert(std::make_pair(name,loc));
                    }
                }
                // insert type locations
                {
                    __types_t__::iterator iter =
                        tmp._types.begin();
                    for ( ;iter!=tmp._types.end();++iter )
                        _types.insert(*iter);
                }
                // insert syscalls locations
                {
                    __references_t__::iterator iter =
                        tmp._system_calls.begin();
                    for ( ;iter!=tmp._system_calls.end();++iter )
                        _system_calls.insert(*iter);
                }

                // merge files
                _text.insert( _text.end(), tmp._text.begin(), tmp._text.end());
                _data.insert( _data.end(), tmp._data.begin(), tmp._data.end());
                _func.insert( _func.end(), tmp._func.begin(), tmp._func.end());

                return;

            }
        }
        printf("File '%s' could not be found\n", yarn_file_name.c_str());
    }
    void Object::reset() {
        _text.clear();
        _data.clear();
        _program.clear();
        _needs_reference.clear();
        _references.clear();
        _types.clear();
        _system_calls.clear();
    }
    void Object::assemble() {
        while ( _stream.good() ) {
            switch( _stream.front().tokt ) {
                case Lexer::token_t::TOKEN:
                    this -> __stmt_opcode();
                    break;
                case Lexer::token_t::SYMBOL:
                    switch (_stream.frontData()[0]) {
                        case '\n':
                            _stream.next();
                            break;
                        case '.':
                            _stream.next();
                            this -> __stmt_assign();
                            break;
                        case ':':
                            _stream.next();
                            this -> __stmt_reference();
                            break;
                        default:
                            sendError("Invalid %s '%s' (%d:%d)",
                                Lexer::token(_stream.front().tokt),
                                _stream.frontData(),
                                _stream.front().line, 
                                _stream.front().col
                                );
                            break;
                    }
                    break;
                case Lexer::token_t::PREFIX:
                    this -> __stmt_metacode();
                    break;
                default:
                    sendError("Invalid %s '%s' (%d:%d)",
                        Lexer::token(_stream.front().tokt),
                        _stream.frontData(),
                        _stream.front().line, 
                        _stream.front().col
                        );
                    break;
            }
        }
    }
    void Object::link() {
        CODE::HEADER hd;

        // post processing
        _func.push_back('\0');
        _data.push_back('\0');
        _text.push_back(CODE::RETN);

        // allocate program size
        Unit prog_size = 
            sizeof(CODE::HEADER) +
            _func.size() + 
            _data.size() +
            _text.size() +
            2 * _kernel.size() * sizeof(Unit);

        _program.clear();
        _program.resize(prog_size);

        //
        // write program header data
        //
        hd.VERSION[0]        = LOOM_MAJOR_VERSION; // MAJOR
        hd.VERSION[1]        = LOOM_MINOR_VERSION; // MINOR
        hd.VERSION[2]        = LOOM_PATCH_VERSION; // PATCH
        hd.FLAGS             = ((_meta_parse_args) ? CODE::HEADER::FLAG_PARSE_ARGS : 0);
        hd.PROG_SIZE         = prog_size;
        hd.CALL_TABLE_OFFSET = sizeof(CODE::HEADER);
        hd.CALL_TABLE_SIZE   = _func.size();
        hd.DATA_OFFSET       = hd.CALL_TABLE_OFFSET + hd.CALL_TABLE_SIZE;
        hd.DATA_SIZE         = _data.size();
        hd.TEXT_OFFSET       = hd.DATA_OFFSET + hd.DATA_SIZE;
        hd.TEXT_SIZE         = _text.size();
        hd.KERNEL_OFFSET     = hd.TEXT_OFFSET + hd.TEXT_SIZE;
        hd.KERNEL_SIZE       = 2 * _kernel.size() * sizeof(Unit);
        hd.ENTRY_POINT       = _meta_entry_point + hd.TEXT_OFFSET;

        // set program header
        *((CODE::HEADER*) &_program[0]) = hd;

        // copy over call table data
        std::copy(_func.begin(),
                  _func.begin() + hd.CALL_TABLE_SIZE,
                  _program.begin() + hd.CALL_TABLE_OFFSET);

        // copy over constant data
        std::copy(_data.begin(),
                  _data.begin() + hd.DATA_SIZE,
                  _program.begin() + hd.DATA_OFFSET);

        // copy over text data
        std::copy(_text.begin(),
                  _text.begin() + hd.TEXT_SIZE,
                  _program.begin() + hd.TEXT_OFFSET);

        // Get a pointer to the location in the TEXTILE where the kernel locations are stored
        Unit* kernel_insert = (Unit*) &_program[hd.KERNEL_OFFSET];

        for ( uint32 i=0;i<_kernel.size();i++ ) {

            // Check to make sure that the kernels only make jump calls
            // to locations within their associated ranges
            for ( uint32 j=_kernel[i].call_start;j<_kernel[i].call_end;j++ ) {
                __references_t__::iterator call = 
                    _references.find(_kernel_calls[j].reference_name);
                if ( call != _references.end() && (call -> second < _kernel[i].text_start || call -> second >= _kernel[i].text_end)) {
                    sendError("Error Linking Program:\nKernel references location outside of kernel range (%d:%d)",
                        _kernel_calls[j].line,
                        _kernel_calls[j].column
                        );
                }
            }

            // Add this kernel to the TEXTILE
            * kernel_insert = _kernel[i].text_start + hd.TEXT_OFFSET;
            * (++ kernel_insert) = _kernel[i].text_end + hd.TEXT_OFFSET;
            ++ kernel_insert ;
        }

        //
        // link all references
        //
        std::set<std::string> unmatched;
        std::set<std::string> unused;

        // fill set with all unused variable
        __references_t__::iterator used = 
            _references.begin();
        for ( ;used!=_references.end(); ++used)
            unused.insert(used -> first);

        __needs_reference_t__::iterator iter =
            _needs_reference.begin();
        for ( ;iter!=_needs_reference.end(); ++iter) {
            const Unit& index = iter -> first;
            const std::string& name = iter -> second;

            Code arg_type;
            Unit arg_value;
            __text_read_argument(_text, arg_type, arg_value, index);

            __references_t__::iterator match =
                _references.find(name);
            if ( match == _references.end() ) {
                fprintf(stderr, "Could not match reference %s\n", name.c_str());
                unmatched.insert(name);
                continue;
            }

            __types_t__::iterator type =
                _types.find(name);
            if ( type == _types.end() ) {
                fprintf(stderr, "Could not find type for reference %s\n", name.c_str());
                unmatched.insert(name);
                continue;
            }
            __allowed_types_t__::iterator allowed =
                _allowed.find(index);
            if ( allowed != _allowed.end() && !(type -> second & allowed -> second)) {
                fprintf(stderr, "Type of reference \"%s\" (%d) does not match used location (%d)\n",
                        name.c_str(),
                        type -> second,
                        arg_type
                        );
                unmatched.insert(name);
                continue;
            }

            arg_value = match -> second;

            if ( arg_type == ASM_TYPE_TEXT ) {
                arg_type = CODE::VALUE;

                // Add the location of text in the TEXTILE to this value
                arg_value += hd.TEXT_OFFSET;
            } else if ( arg_type == ASM_TYPE_DATA ) {
                arg_type = CODE::DATA;

                // Add the location of data in the TEXTILE to this value
                arg_value += hd.DATA_OFFSET;
            }
            else if ( arg_type == ASM_TYPE_VALUE )
                arg_type = CODE::VALUE;
            else if ( arg_type == ASM_TYPE_LOCAL )
                arg_type = CODE::LOCAL;
            else if ( arg_type == ASM_TYPE_ADDR )
                arg_type = CODE::ADDR;
            else if ( arg_type == ASM_TYPE_GLOB )
                arg_type = CODE::GLOB;

            __text_write_argument(_program, arg_type, arg_value, index + hd.TEXT_OFFSET);

            std::set<std::string>::const_iterator iter = 
                unused.find(name);
            if ( iter != unused.end() )
                unused.erase(iter);

        }

        if ( !unmatched.empty() ) {
            std::string errors;
            for ( std::set<std::string>::iterator i=unmatched.begin();i!=unmatched.end();++i )
                errors += "    Unmatched reference \"" + (*i) + "\"\n";
            sendError("Error(s) Linking Program:\n%s", errors.c_str());
        }

        if ( !unused.empty() ) {
            std::string warnings = "Warning(s) Linking Program:\n";
            for ( std::set<std::string>::iterator i=unused.begin();i!=unused.end();++i )
                warnings += "    Unused variable \"" + (*i) + "\"\n";
            printf("%s", warnings.c_str());
        }
    }

    const std::vector<uint8_t>& Object::getProgram() const {
        return _program;
    }

    void Object::__set_reference(const std::string& name, Unit value) {
        __references_t__::const_iterator match =
            _references.find(name);
        if ( match != _references.end() )
            sendError("Duplicate reference %s (%d:%d)",
                name.c_str(),
                _stream.front().line,
                _stream.front().col
                );

        _references.insert(match, std::make_pair(name, value));
    }
    void Object::__set_type(const std::string& name, Code code) {
        __types_t__::const_iterator match =
            _types.find(name);
        if ( match != _types.end() )
            sendError("Duplicate type for %s (%d:%d)",
                name.c_str(),
                _stream.front().line,
                _stream.front().col
                );

        _types.insert(match, std::make_pair(name, code));
    }

    const std::string& Object::__get_anon_varname() {
        if ( _anon_varname.empty() ) {
            _anon_varname = "+a";
        } else if ( _anon_varname[_anon_varname.size()-1] == 'z' ) {
            for ( Unit i=1;i<_anon_varname.size();i++ ) {
                _anon_varname[i] = 'a';
            }
            _anon_varname += 'a';
        } else {
            ++ _anon_varname[_anon_varname.size()-1];
        }
        return _anon_varname;
    }
    bool Object::__token_is_variable() {
        return (_stream.frontData()) ? 
            _stream.getInternal().isAlpha(_stream.frontData()[0]) :
            false;
    }
    Unit Object::__get_token_value() {
        Unit result = 0;
        parse_value(_stream, result, PARSE_BOTH);
        return result;
    }
    void Object::__meta_search() {
        need(Lexer::token_t::STRING, _stream);
        printf("Search in directory '%s'\n", _stream.frontData());
        _meta_search.push_back(_stream.frontData());
        _stream.next();
        need(Lexer::token_t::SYMBOL, "\n", _stream);
        _stream.next();
    }
    void Object::__meta_use() {
        need(Lexer::token_t::STRING, _stream);
        printf("Include file '%s'\n", _stream.frontData());
        std::string include_location = _stream.frontData();
        _stream.next();
        need(Lexer::token_t::SYMBOL, "\n", _stream);
        _stream.next();

        this -> include(include_location);
    }
    void Object::__meta_parse_args() {
        need(Lexer::token_t::SYMBOL, "\n", _stream);
        if ( _meta_parse_args ) {
            sendError("Duplicate 'parse_args' option found in file (%d:%d)",
                _stream.front().line,
                _stream.front().col
                );
        }
        printf("This textile will parse arguments\n");
        _meta_parse_args = true;
        _stream.next();
    }
    void Object::__meta_entry() {
        need(Lexer::token_t::SYMBOL, "\n", _stream);
        if ( _meta_entry_point != (uint32) -1 ) {
            sendError("Duplicate 'entry' option found in file (%d:%d)",
                _stream.front().line,
                _stream.front().col
                );
        }
        printf("Entry point for textile is 0x%x\n", (unsigned)( _text.size() + sizeof(CODE::HEADER) ) );
        _meta_entry_point = _text.size();
        _stream.next();
    }
    void Object::__meta_push_function() {
        need(Lexer::token_t::SYMBOL, "\n", _stream);
        printf("Pushing new function at 0x%x\n", (unsigned) _meta_stack_pointers.top());
        _meta_stack_pointers.push(_meta_stack_pointers.top());
        _stream.next();
    }
    void Object::__meta_pop_function() {
        need(Lexer::token_t::SYMBOL, "\n", _stream);
        _meta_stack_pointers.pop();
        printf("Popping stack to 0x%x\n", (unsigned) _meta_stack_pointers.top());
        _stream.next();
    }
    void Object::__meta_kernel() {
        if ( _in_kernel_mode ) 
            sendError("Cannot enter kernel mode while already in kernel mode (%d:%d)",
                _stream.front().line,
                _stream.front().col
                );
        printf("Pushing new function at 0x%x\n", (unsigned) _meta_stack_pointers.top());
        _meta_stack_pointers.push(_meta_stack_pointers.top());

        need(Lexer::token_t::TOKEN, _stream);
        std::string reference = _stream.frontData();

        __set_reference( reference, _text.size() );
        __set_type( reference, ASM_TYPE_TEXT );

        _kernel.push_back({_text.size(), 0, _kernel_calls.size(), 0});
        _in_kernel_mode = true;
        _stream.next();

        need(Lexer::token_t::SYMBOL, "\n", _stream);
        _stream.next();
    }
    void Object::__meta_end_kernel() {
        if ( !_in_kernel_mode ) 
            sendError("Unmatched @end_kernel (%d:%d)",
                _stream.front().line,
                _stream.front().col
                );

        _meta_stack_pointers.pop();
        printf("Popping stack to 0x%x\n", (unsigned) _meta_stack_pointers.top());

        __kernel_block__& kb = _kernel.back();
        kb.text_end = _text.size();
        kb.call_end = _kernel_calls.size();
        _in_kernel_mode = false;
    }
    void Object::__stmt_opcode() {
        typedef void (Object::*opcode_function_ptr)();
        typedef std::unordered_map<std::string, opcode_function_ptr> function_map;
        static function_map _opcode_map = {
            {"NOOP", &Object::__noop},
            {"DEBUG", &Object::__debug},
            {"ASSG", &Object::__assg},
            {"CALL", &Object::__call},
            {"SYSC", &Object::__sysc},
            {"RETN", &Object::__retn},
            {"PUSH", &Object::__push},
            {"POP", &Object::__pop},
            {"MAKE", &Object::__make},
            {"FREE", &Object::__free},
            {"JUMP", &Object::__jump},
            {"JEQ", &Object::__jeq},
            {"JNEQ", &Object::__jneq},
            {"JLS", &Object::__jls},
            {"JLSE", &Object::__jlse},
            {"JGR", &Object::__jgr},
            {"JGRE", &Object::__jgre},
            {"JZ", &Object::__jz},
            {"JNZ", &Object::__jnz},
            {"INCR", &Object::__incr},
            {"DECR", &Object::__decr},
            {"INEG", &Object::__ineg},
            {"IADD", &Object::__iadd},
            {"ISUB", &Object::__isub},
            {"IMUL", &Object::__imul},
            {"IDIV", &Object::__idiv},
            {"IPOW", &Object::__ipow},
            {"IMOD", &Object::__imod},
            {"ISQRT", &Object::__isqrt},
            {"ILOG", &Object::__ilog},
            {"SIN", &Object::__sin},
            {"COS", &Object::__cos},
            {"TAN", &Object::__tan},
            {"FNEG", &Object::__fneg},
            {"FADD", &Object::__fadd},
            {"FSUB", &Object::__fsub},
            {"FMUL", &Object::__fmul},
            {"FDIV", &Object::__fdiv},
            {"FPOW", &Object::__fpow},
            {"FMOD", &Object::__fmod},
            {"FSQRT", &Object::__fsqrt},
            {"FLOG", &Object::__flog},
            {"FVNEG", &Object::__fvneg},
            {"FVADD", &Object::__fvadd},
            {"FVSUB", &Object::__fvsub},
            {"FVMUL", &Object::__fvmul},
            {"FVDIV", &Object::__fvdiv},
            {"FVDOT", &Object::__fvdot},
            {"FVCROSS", &Object::__fvcross},
            {"MNEG", &Object::__mneg},
            {"MADD", &Object::__madd},
            {"MSUB", &Object::__msub},
            {"MMUL", &Object::__mmul},
            {"MTRANS", &Object::__mtrans},
            {"RPC", &Object::__rpc},
            {"RPCS", &Object::__rpcs},
            {"KERN1", &Object::__kernel1},
            {"KERN2", &Object::__kernel2},
            {"KERN3", &Object::__kernel3},
            {"REDC", &Object::__reduction},
            {"SCAN", &Object::__scan},
            {"SORT", &Object::__sort},
            {"SSORT", &Object::__stable_sort},
            {"START", &Object::__start},
            {"STOP", &Object::__stop},
            {"SYNC", &Object::__sync},
            {"CNFG1", &Object::__config1},
            {"CNFG2", &Object::__config2},
            {"CNFG3", &Object::__config3},
            {"COPY", &Object::__copy},
            {"BUFFER", &Object::__buffer},
            {"INDEX1", &Object::__index1},
            {"INDEX2", &Object::__index2},
            {"INDEX3", &Object::__index3},
            {"KARG", &Object::__karg},
            {"FILEOPEN", &Object::__file_open},
            {"FILECLOSE", &Object::__file_close},
            {"FILEREAD", &Object::__file_read},
            {"FILEWRITE", &Object::__file_write},
            {"FILESEEK", &Object::__file_seek},
            {"FILETELL", &Object::__file_tell},
            {"FILERESET", &Object::__file_reset},
            {"PRINTF", &Object::__printf},
            {"FPRINTF", &Object::__fprintf},
            {"MEMCPY", &Object::__memcpy},
        };

        function_map::iterator itr = _opcode_map.find(_stream.frontData());
        if ( itr == _opcode_map.end() ) {
            sendError("Unknown opcode '%s' (%d:%d)",
                _stream.frontData(),
                _stream.front().line,
                _stream.front().col
                );
        }

        _stream.next();
        opcode_function_ptr f = itr -> second;
        ((*this).*f)();

        need(Lexer::token_t::SYMBOL, "\n", _stream);
        _stream.next();
    }
    void Object::__stmt_metacode() {
        typedef void (Object::*meta_function_ptr)();
        typedef std::unordered_map<std::string, meta_function_ptr> function_map; 
        static function_map _metacode_map = {
            {"search", &Object::__meta_search},
            {"use", &Object::__meta_use},
            {"parse_args", &Object::__meta_parse_args},
            {"entry", &Object::__meta_entry},
            {"function", &Object::__meta_push_function},
            {"end_function", &Object::__meta_pop_function},
            {"kernel", &Object::__meta_kernel},
            {"end_kernel", &Object::__meta_end_kernel},
        };

        function_map::iterator itr = _metacode_map.find(_stream.frontData());
        if ( itr == _metacode_map.end() ) {
            sendError("Unknown keyword '%s' (%d:%d)",
                _stream.frontData(),
                _stream.front().line,
                _stream.front().col
                );
        }

        _stream.next();
        
        meta_function_ptr f = itr -> second;
        ((*this).*f)();
    }
    void Object::__stmt_reference() {
        need(Lexer::token_t::TOKEN, _stream);
        std::string reference = _stream.frontData();

        __set_reference( reference, _text.size() );
        __set_type( reference, ASM_TYPE_TEXT );

        _stream.next();
        need(Lexer::token_t::SYMBOL, "\n", _stream);
        _stream.next();
    }
    void Object::__stmt_assign() {
        need(Lexer::token_t::TOKEN, _stream);
        std::string variable = _stream.frontData();
        _stream.next();
        need(Lexer::token_t::SYMBOL, "=", _stream);
        _stream.next();

        // integer
        if ( want(Lexer::token_t::TOKEN, _stream) || want(Lexer::token_t::SYMBOL, "-", _stream) ) {
            Unit result;
            parse_value(_stream, result, PARSE_BOTH);
            printf("%s = %lu\n", variable.c_str(), result);

            __set_reference( variable, result );
            __set_type( variable, ASM_TYPE_VALUE );
        }

        // string
        else if ( want(Lexer::token_t::STRING, _stream) ) {
            std::string str = _stream.frontData();
            _stream.next();
            printf("%s = \"%s\"\n", variable.c_str(), str.c_str());

            __references_t__::iterator find_string =
                _string_map.find(str);
            if ( find_string == _string_map.end() )  {
                _string_map.insert(find_string, std::make_pair(str, _data.size()));
                __set_reference( variable, _data.size() );
                for ( const char* i=str.c_str();
                      *i != '\0'; i++ ) {
                    _data.push_back(*i);
                }
                _data.push_back('\0');
            } else {
                __set_reference( variable, find_string -> second );
            }
            __set_type( variable, ASM_TYPE_DATA );
        }

        // special operator
        else {
            need(Lexer::token_t::PREFIX, _stream);
            std::string name = _stream.frontData();

            if ( name == "stack_size" ) {
                
                _stream.next();
                printf("%s = %lu\n", variable.c_str(), _meta_stack_pointers.top());
                __set_reference( variable, _meta_stack_pointers.top() );
                __set_type( variable, ASM_TYPE_VALUE );

            } else if ( name == "stack" ) {
                _stream.next();
                printf("%s = %lu\n", variable.c_str(), _meta_stack_pointers.top());

                __set_reference(variable, _meta_stack_pointers.top());
                __set_type( variable, ASM_TYPE_VALUE );
                
                need(Lexer::token_t::PREFIX, _stream);
                std::string type = _stream.frontData();

                uint32 count;

                if ( type == "unit" )
                    count = sizeof(Unit);
                else if ( type == "address" )
                    count = sizeof(void*);
                else if ( type == "sync" )
                    count = sizeof(CODE::Sync*);
                else
                    sendError("Invalid stack type '%s' (%d:%d)",
                        type.c_str(),
                        _stream.front().line,
                        _stream.front().col
                        );

                _stream.next();

                if ( want(Lexer::token_t::OPEN_BRACKET, _stream) ) {
                    _stream.next();
                    
                    Unit value;
                    parse_value(_stream, value, PARSE_INT);
                    count *= value;

                    need(Lexer::token_t::CLOSE_BRACKET, _stream);
                    _stream.next();
                }

                _meta_stack_pointers.top() += count;

            } else {
                sendError("Unknown operator '%s' (%d:%d)",
                    name.c_str(),
                    _stream.front().line,
                    _stream.front().col
                    );
            }
        }

        need(Lexer::token_t::SYMBOL, "\n", _stream);
        _stream.next();
    }
    void Object::__text_code(Code item) {
        Unit index = _text.size();
        _text.resize(_text.size() + sizeof(Code));
        *((Code*)&_text[index]) = item;
    }
    void Object::__text_unit(Unit item) {
        Unit index = _text.size();
        _text.resize(_text.size() + sizeof(Unit));
        *((Unit*)&_text[index]) = item;
    }
    void Object::__text_argument(Code type, Unit value) {
        Unit index = _text.size();
        _text.resize(_text.size() + sizeof(Code) + sizeof(Unit));
        __text_write_argument(_text, type, value, index);;
    }
    void Object::__text_write_argument(std::vector<uint8_t>& text, Code type, Unit value, Unit index) {
        *((Code*)&text[index]) = type;
        *((Unit*)&text[index+sizeof(Code)]) = value;
        //fprintf(stderr, "write %u, %lu to %lu\n", arg.type, arg.value, index);
    }
    void Object::__text_read_argument(std::vector<uint8_t>& text, Code& type, Unit& value, Unit index) {
        type = *((Code*)&text[index]);
        value = *((Unit*)&text[index+sizeof(Code)]);
        //fprintf(stderr, "read %u, %lu to %lu\n", arg.type, arg.value, index);
    }
    
}
