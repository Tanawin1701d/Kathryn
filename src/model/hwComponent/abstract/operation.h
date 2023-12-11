//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_OPERATION_H
#define KATHRYN_OPERATION_H

#include <string>
#include <cassert>

namespace kathryn {
    enum LOGIC_OP {
        /** bitwise operators*/
        BITWISE_AND,  // &
        BITWISE_OR,   // |
        BITWISE_XOR,  // ^
        BITWISE_INVR, // ~
        BITWISE_SHL,  // <<
        BITWISE_SHR,  // >>
        /** logical operators*/
        LOGICAL_AND,  // &&
        LOGICAL_OR,   // ||
        LOGICAL_NOT,  // !
        /** relational operator*/
        RELATION_EQ,  // ==
        RELATION_NEQ, // !=
        RELATION_LE,  // <
        RELATION_LEQ, // <=
        RELATION_GE,  // >
        RELATION_GEQ, // >=
        /** arithmaktic operators*/
        ARITH_PLUS,   // +
        ARITH_MINUS,  // -
        ARITH_MUL,    // *
        ARITH_DIV,    // /
        ARITH_DIVR,   // %
        /** assign for build new variable*/
        ASSIGN,       // =
        /** other operator*/
        DUMMY,
        LOGIC_OP_COUNT
    };

    std::string lop_to_string(LOGIC_OP op);

    const int LOGICAL_SIZE = 1;

}
#endif //KATHRYN_OPERATION_H
