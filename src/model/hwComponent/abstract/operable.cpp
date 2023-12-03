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
    expression& Operable::operator&(Operable &b) {
        auto ret =  std::make_shared<expression>(BITWISE_AND,
                                            std::shared_ptr<Operable>(&this->getExactOperable()),
                                            this->getOperableSlice(),
                                            std::shared_ptr<Operable>(&b.getExactOperable()),
                                            b.getOperableSlice(),
                                            getOperableSlice().getSize());
        
        return *ret;
    }

    expression& Operable::operator|(Operable &b) {
        auto ret =  std::make_shared<expression>(BITWISE_OR,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator^(Operable &b) {
        auto ret =  std::make_shared<expression>(BITWISE_XOR,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator~() {
        auto ret =  std::make_shared<expression>(BITWISE_INVR,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 nullptr,
                                                 Slice(),
                                                 getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator<<(Operable &b) {
        auto ret =  std::make_shared<expression>(BITWISE_SHL,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator>>(Operable &b) {
        auto ret =  std::make_shared<expression>(BITWISE_SHR,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 getOperableSlice().getSize());

        return *ret;
    }


    /** logical operators*/

    expression& Operable::operator&&(Operable &b) {
        auto ret =  std::make_shared<expression>(LOGICAL_AND,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator||(Operable &b) {
        auto ret =  std::make_shared<expression>(LOGICAL_OR,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!() {
        auto ret =  std::make_shared<expression>(LOGICAL_NOT,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 nullptr,
                                                 Slice(),
                                                 LOGICAL_SIZE);

        return *ret;
    }

    /** relational operator*/

    expression& Operable::operator==(Operable &b) {
        auto ret =  std::make_shared<expression>(RELATION_EQ,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!=(Operable &b) {
        auto ret =  std::make_shared<expression>(RELATION_NEQ,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<(Operable &b) {
        auto ret =  std::make_shared<expression>(RELATION_LE,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<=(Operable &b) {
        auto ret =  std::make_shared<expression>(RELATION_LEQ,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>(Operable &b) {
        auto ret =  std::make_shared<expression>(RELATION_GE,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>=(Operable &b) {
        auto ret =  std::make_shared<expression>(RELATION_GEQ,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                                 LOGICAL_SIZE);

        return *ret;
    }

    /** arithmetic operators*/

    expression& Operable::operator+(Operable &b) {
        auto ret =  std::make_shared<expression>(ARITH_PLUS,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                            getOperableSlice().getSize() + 1);
        /** size + 1 because we provide carry for expression*/

        return *ret;
    }

    expression& Operable::operator-(Operable &b) {
        auto ret =  std::make_shared<expression>(ARITH_MINUS,
                                             std::shared_ptr<Operable>(&this->getExactOperable()),
                                             this->getOperableSlice(),
                                             std::shared_ptr<Operable>(&b.getExactOperable()),
                                             b.getOperableSlice(),
                                            getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator*(Operable &b) {
        auto ret =  std::make_shared<expression>(ARITH_MUL,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                            getOperableSlice().getSize() * b.getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator/(Operable &b) {
        auto ret =  std::make_shared<expression>(ARITH_DIV,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                            getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator%(Operable &b) {
        auto ret =  std::make_shared<expression>(ARITH_DIV,
                                                 std::shared_ptr<Operable>(&this->getExactOperable()),
                                                 this->getOperableSlice(),
                                                 std::shared_ptr<Operable>(&b.getExactOperable()),
                                                 b.getOperableSlice(),
                                            getOperableSlice().getSize());

        return *ret;
    }

}