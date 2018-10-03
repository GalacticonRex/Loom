#include "assembler.hpp"

namespace Loom {

    const uint32 Object::PARSE_INT = 0x01;
    const uint32 Object::PARSE_FLOAT = 0x02;
    const uint32 Object::PARSE_BOTH = 0x03;

    const Code Object::ASM_TYPE_VALUE = 0x01;
    const Code Object::ASM_TYPE_TEXT  = 0x02;
    const Code Object::ASM_TYPE_DATA  = 0x04;
    const Code Object::ASM_TYPE_LOCAL = 0x08;
    const Code Object::ASM_TYPE_ADDR  = 0x10;
    const Code Object::ASM_TYPE_GLOB  = 0x20;
    const Code Object::ASM_TYPE_LITERAL = 0x07;
    const Code Object::ASM_TYPE_VARIABLE = 0x38;
    const Code Object::ASM_TYPE_ANY = 0xFF;
    
}