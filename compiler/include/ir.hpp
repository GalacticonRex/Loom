/*#ifndef __LOOM_INTERNAL_REPRESENTATION_HPP__
#define __LOOM_INTERNAL_REPRESENTATION_HPP__

#include <vector>
#include "type.hpp"

namespace Loom {

    struct IR {
        enum Code {
            OUT,
            INVOKE,
            FUNC,
            BRANCH,
            COND,
            TUPLE,
            MOD,
            PRED,
            SUCC,
            SQRT
        };

    private:

        std::vector<ubyte> _data;

    public:

        IR(std::ifstream& input);
        
    };

};

#endif//__LOOM_INTERNAL_REPRESENTATION_HPP__*/
