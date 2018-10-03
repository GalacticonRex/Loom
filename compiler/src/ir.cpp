/*#include "ir.hpp"
#include <stack>
#include <unordered_map>

namespace Loom {

    struct IRReq {
        Unit loc;
        Unit args;
    };

    typedef std::vector<ubyte> IRData;
    typedef std::stack<IRReq> IRStack;
    typedef std::unordered_map<std::string, Unit> IRLinkTo;
    typedef std::unordered_map<Unit, std::string> IRLinkFrom;
    typedef std::unordered_map<std::string, Unit> IRTable;

    struct IRState {
        IRData& data;
        IRState& stack;
        IRLinkTo& linkTo;
        IRLinkFrom& linkFrom;
    }

    static void IR_out(IRState);

    IR::IR(std::ifstream& input) {
        static IRTable ops = {

        };

        IRStack stack;
        IRLinkTo linkTo;
        IRLinkFrom linkFrom;
        IRState st{_data, stack, linkTo, linkFrom};

        while ( input.good() ) {
            std::string node;
            input >> node;


        }
    }

}
*/
