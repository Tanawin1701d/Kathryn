//
// Created by tanawin on 29/11/2566.
//

#include"operable.h"
#include "model/hwComponent/expression/expression.h"

namespace kathryn {

/***
     * todo future,  we will make it handle case unequal size
     *
     * */

/** bitwise operators*/
    expression& Operable::operator&(const Operable &b) {
        auto ret =  new expression(BITWISE_AND,
                                   this,
                                   &b,
                                   getOperableSlice().getSize());
        
        return *ret;
    }

    expression& Operable::operator|(const Operable &b) {
        auto ret =  new expression(BITWISE_OR,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator^(const Operable &b) {
        auto ret =  new expression(BITWISE_XOR,
                                    this,
                                    &b,
                                    getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator~() {
        auto ret =  new expression(BITWISE_INVR,
                                     this,
                                     nullptr,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator<<(const Operable &b) {
        auto ret =  new expression(BITWISE_SHL,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator>>(const Operable &b) {
        auto ret =  new expression(BITWISE_SHR,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }


    /** logical operators*/

    expression& Operable::operator&&(const Operable &b) {
        auto ret =  new expression(LOGICAL_AND,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator||(const Operable &b) {
        auto ret =  new expression(LOGICAL_OR,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!() {
        auto ret =  new expression(LOGICAL_NOT,
                                     this,
                                     nullptr,
                                     LOGICAL_SIZE);

        return *ret;
    }

    /** relational operator*/

    expression& Operable::operator==(const Operable &b) {
        auto ret =  new expression(RELATION_EQ,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!=(const Operable &b) {
        auto ret =  new expression(RELATION_NEQ,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<(const Operable &b) {
        auto ret =  new expression(RELATION_LE,
                                    this,
                                    &b,
                                    LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<=(const Operable &b) {
        auto ret =  new expression(RELATION_LEQ,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>(const Operable &b) {
        auto ret =  new expression(RELATION_GE,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>=(const Operable &b) {
        auto ret =  new expression(RELATION_GEQ,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    /** arithmetic operators*/

    expression& Operable::operator+(const Operable &b) {
        auto ret =  new expression(ARITH_PLUS,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());
        /** size + 1 because we provide carry for exprMetas*/

        return *ret;
    }

    expression& Operable::operator-(const Operable &b) {
        auto ret =  new expression(ARITH_MINUS,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator*(const Operable &b) {
        auto ret =  new expression(ARITH_MUL,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator/(const Operable &b) {
        auto ret =  new expression(ARITH_DIV,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator%(const Operable &b) {
        auto ret =  new expression(ARITH_DIVR,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

}