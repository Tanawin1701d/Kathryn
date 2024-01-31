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
                                    &this->getExactOperable(),
                                    &b.getExactOperable(),
                                    getOperableSlice().getSize());
        
        return *ret;
    }

    expression& Operable::operator|(const Operable &b) {
        auto ret =  new expression(BITWISE_OR,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator^(const Operable &b) {
        auto ret =  new expression(BITWISE_XOR,
                                    &this->getExactOperable(),
                                    &b.getExactOperable(),
                                    getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator~() {
        auto ret =  new expression(BITWISE_INVR,
                                     &this->getExactOperable(),
                                     nullptr,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator<<(const Operable &b) {
        auto ret =  new expression(BITWISE_SHL,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator>>(const Operable &b) {
        auto ret =  new expression(BITWISE_SHR,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     getOperableSlice().getSize());

        return *ret;
    }


    /** logical operators*/

    expression& Operable::operator&&(const Operable &b) {
        auto ret =  new expression(LOGICAL_AND,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator||(const Operable &b) {
        auto ret =  new expression(LOGICAL_OR,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!() {
        auto ret =  new expression(LOGICAL_NOT,
                                     &this->getExactOperable(),
                                     nullptr,
                                     LOGICAL_SIZE);

        return *ret;
    }

    /** relational operator*/

    expression& Operable::operator==(const Operable &b) {
        auto ret =  new expression(RELATION_EQ,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!=(const Operable &b) {
        auto ret =  new expression(RELATION_NEQ,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<(const Operable &b) {
        auto ret =  new expression(RELATION_LE,
                                    &this->getExactOperable(),
                                    &b.getExactOperable(),
                                    LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<=(const Operable &b) {
        auto ret =  new expression(RELATION_LEQ,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>(const Operable &b) {
        auto ret =  new expression(RELATION_GE,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>=(const Operable &b) {
        auto ret =  new expression(RELATION_GEQ,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    /** arithmetic operators*/

    expression& Operable::operator+(const Operable &b) {
        auto ret =  new expression(ARITH_PLUS,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     getOperableSlice().getSize());
        /** size + 1 because we provide carry for exprMetas*/

        return *ret;
    }

    expression& Operable::operator-(const Operable &b) {
        auto ret =  new expression(ARITH_MINUS,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator*(const Operable &b) {
        auto ret =  new expression(ARITH_MUL,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator/(const Operable &b) {
        auto ret =  new expression(ARITH_DIV,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator%(const Operable &b) {
        auto ret =  new expression(ARITH_DIVR,
                                     &this->getExactOperable(),
                                     &b.getExactOperable(),
                                     getOperableSlice().getSize());

        return *ret;
    }

}