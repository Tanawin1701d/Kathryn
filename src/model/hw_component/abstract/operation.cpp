//
// Created by tanawin on 11/12/2566.
//

#include "operation.h"

namespace kathryn{

    std::string lop_to_string(LOGIC_OP op){
        std::string mapper[LOGIC_OP_COUNT] = {
                "&", "|", "^", "~", "<<",
                ">>", "&&", "||", "!", "==",
                "!=", "<", "<=", ">", ">=",
                "+", "-", "*", "/", "%",
                "=", "DUMMYOP"
        };
        assert(op < LOGIC_OP_COUNT);
        return mapper[op];
    }

    bool is_single_opr(LOGIC_OP op){
        return (op == ASSIGN) || (op == BITWISE_INVR) || (op == LOGICAL_NOT);
    }

    bool is_double_opr(LOGIC_OP op){
        return !is_single_opr(op);
    }

}