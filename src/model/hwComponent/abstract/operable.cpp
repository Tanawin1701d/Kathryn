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
                                    this->getOperableSlice(),
                                    &b.getExactOperable(),
                                    b.getOperableSlice(),
                                    getOperableSlice().getSize());
        
        return *ret;
    }

    expression& Operable::operator|(const Operable &b) {
        auto ret =  new expression(BITWISE_OR,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator^(const Operable &b) {
        auto ret =  new expression(BITWISE_XOR,
                                    &this->getExactOperable(),
                                    this->getOperableSlice(),
                                    &b.getExactOperable(),
                                    b.getOperableSlice(),
                                    getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator~() {
        auto ret =  new expression(BITWISE_INVR,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     nullptr,
                                     Slice(),
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator<<(const Operable &b) {
        auto ret =  new expression(BITWISE_SHL,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator>>(const Operable &b) {
        auto ret =  new expression(BITWISE_SHR,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     getOperableSlice().getSize());

        return *ret;
    }


    /** logical operators*/

    expression& Operable::operator&&(const Operable &b) {
        auto ret =  new expression(LOGICAL_AND,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator||(const Operable &b) {
        auto ret =  new expression(LOGICAL_OR,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!() {
        auto ret =  new expression(LOGICAL_NOT,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     nullptr,
                                     Slice(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    /** relational operator*/

    expression& Operable::operator==(const Operable &b) {
        auto ret =  new expression(RELATION_EQ,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!=(const Operable &b) {
        auto ret =  new expression(RELATION_NEQ,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<(const Operable &b) {
        auto ret =  new expression(RELATION_LE,
                                    &this->getExactOperable(),
                                    this->getOperableSlice(),
                                    &b.getExactOperable(),
                                    b.getOperableSlice(),
                                    LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<=(const Operable &b) {
        auto ret =  new expression(RELATION_LEQ,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>(const Operable &b) {
        auto ret =  new expression(RELATION_GE,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>=(const Operable &b) {
        auto ret =  new expression(RELATION_GEQ,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     LOGICAL_SIZE);

        return *ret;
    }

    /** arithmetic operators*/

    expression& Operable::operator+(const Operable &b) {
        auto ret =  new expression(ARITH_PLUS,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     getOperableSlice().getSize());
        /** size + 1 because we provide carry for exprMetas*/

        return *ret;
    }

    expression& Operable::operator-(const Operable &b) {
        auto ret =  new expression(ARITH_MINUS,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator*(const Operable &b) {
        auto ret =  new expression(ARITH_MUL,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator/(const Operable &b) {
        auto ret =  new expression(ARITH_DIV,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator%(const Operable &b) {
        auto ret =  new expression(ARITH_DIVR,
                                     &this->getExactOperable(),
                                     this->getOperableSlice(),
                                     &b.getExactOperable(),
                                     b.getOperableSlice(),
                                     getOperableSlice().getSize());

        return *ret;
    }

}