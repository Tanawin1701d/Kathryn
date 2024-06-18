//
// Created by tanawin on 29/11/2566.
//

#include"operable.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/value/value.h"


namespace kathryn {

/***
     * todo future,  we will make it handle case unequal size
     *
     * */

/** bitwise operators*/
    expression& Operable::operator&( Operable &b) {
        mfAssert(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
                 "operable<&> get mismatch bit size"
                 );
        auto ret =  new expression(BITWISE_AND,
                                   this,
                                   &b,
                                   getOperableSlice().getSize());
        
        return *ret;
    }

    expression& Operable::operator|(Operable &b) {
        mfAssert(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
                 "operable<|> get mismatch bit size"
        );
        auto ret =  new expression(BITWISE_OR,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator^( Operable &b) {
        mfAssert(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
                 "operable<^> get mismatch bit size"
        );
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

    expression& Operable::operator<<( Operable &b) {
        mfAssert(b.getOperableSlice().getSize() <= bitSizeOfUll,
                 "operable<&> get mismatch bit size"
        );
        auto ret =  new expression(BITWISE_SHL,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator>>( Operable &b) {
        mfAssert(b.getOperableSlice().getSize() <= bitSizeOfUll,
                 "operable<&> get mismatch bit size"
        );
        auto ret =  new expression(BITWISE_SHR,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }


    /** logical operators*/

    expression& Operable::operator&&( Operable &b) {
        auto ret =  new expression(LOGICAL_AND,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator||( Operable &b) {
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

    expression& Operable::operator==( Operable &b) {
        auto ret =  new expression(RELATION_EQ,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!=( Operable &b) {
        auto ret =  new expression(RELATION_NEQ,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<( Operable &b) {
        auto ret =  new expression(RELATION_LE,
                                    this,
                                    &b,
                                    LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<=( Operable &b) {
        auto ret =  new expression(RELATION_LEQ,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>( Operable &b) {
        auto ret =  new expression(RELATION_GE,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>=( Operable &b) {
        auto ret =  new expression(RELATION_GEQ,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    /** arithmetic operators*/

    expression& Operable::operator+( Operable &b) {
        mfAssert(getOperableSlice().getSize() >= b.getOperableSlice().getSize(),
                 "operable<+> get mismatch bit size"
        );
        auto ret =  new expression(ARITH_PLUS,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());
        /** size + 1 because we provide carry for exprMetas*/

        return *ret;
    }

    expression& Operable::operator-( Operable &b) {
        mfAssert(getOperableSlice().getSize() >= b.getOperableSlice().getSize(),
                 "operable<-> get mismatch bit size"
        );
        auto ret =  new expression(ARITH_MINUS,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator*( Operable &b) {
        mfAssert(getOperableSlice().getSize() >= b.getOperableSlice().getSize(),
                 "operable<*> get mismatch bit size"
        );
        auto ret =  new expression(ARITH_MUL,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator/( Operable &b) {
        mfAssert(getOperableSlice().getSize() >= b.getOperableSlice().getSize(),
                 "operable</> get mismatch bit size"
        );
        auto ret =  new expression(ARITH_DIV,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator%( Operable &b) {
        mfAssert(getOperableSlice().getSize() >= b.getOperableSlice().getSize(),
                 "operable<%> get mismatch bit size"
        );
        auto ret =  new expression(ARITH_DIVR,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::extB(int desSize){
        mfAssert(desSize > 0, "desSize must greater than 0");
        mfAssert(getOperableSlice().getSize() == 1, "src extend bit must size must eeq to 1");
        auto ret = new expression(EXTEND_BIT,this, nullptr, desSize);
        return *ret;
    }



    Operable& Operable::getMatchOperable( ull value) const {
            makeVal(optUserAutoVal, getOperableSlice().getSize(), value);
            /** todo check bit size */
            return optUserAutoVal;
    }

    Operable::operator ull(){
        LogicSimEngine* simEngine = getLogicSimEngineFromOpr();
        mfAssert(getAssignMode() == AM_SIM, "can't retrieve data in model building mode");
        mfAssert(simEngine != nullptr, "get value fail");
        return simEngine->getProxyRep()->getVal();
    }
    Operable::operator ValRepBase(){
        mfAssert(getAssignMode() == AM_SIM, "can't retrieve data in model building mode");
        LogicSimEngine* simEngine = getLogicSimEngineFromOpr();
        return *simEngine->getProxyRep();
    }



}