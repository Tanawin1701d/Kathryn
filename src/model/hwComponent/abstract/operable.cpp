//
// Created by tanawin on 29/11/2566.
//

#include"operable.h"

#include <regex>
#include <model/hwComponent/expression/nest.h>

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
        // mfAssert(b.getOperableSlice().getSize() <= bitSizeOfUll,
        //          "operable<&> get mismatch bit size"
        // );
        std::cout << "warning there is shift bit that greater than 64" << std::endl;
        auto ret =  new expression(BITWISE_SHL,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator>>( Operable &b) {
        // mfAssert(b.getOperableSlice().getSize() <= bitSizeOfUll,
        //          "operable<&> get mismatch bit size"
        // );
        std::cout << "warning there is shift bit that greater than 64" << std::endl;
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

    expression& Operable::slt(Operable& b){
        auto ret = new expression(RELATION_SLT,
                                    this,
                                    &b,
                                    LOGICAL_SIZE);
        return *ret;
    }

    expression& Operable::sgt(Operable& b){
        auto ret = new expression(RELATION_SGT,
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

        Operable* actualB = b.uextIfSizeNotEq(getOperableSlice().getSize());

        auto ret =  new expression(ARITH_PLUS,
                                     this,
                                     actualB,
                                     getOperableSlice().getSize());
        /** size + 1 because we provide carry for exprMetas*/

        return *ret;
    }

    expression& Operable::operator-( Operable &b) {
        mfAssert(getOperableSlice().getSize() >= b.getOperableSlice().getSize(),
                 "operable<-> get mismatch bit size"
        );
        Operable* actualB = b.uextIfSizeNotEq(getOperableSlice().getSize());
        auto ret =  new expression(ARITH_MINUS,
                                     this,
                                     actualB,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator*( Operable &b) {
        mfAssert(getOperableSlice().getSize() >= b.getOperableSlice().getSize(),
                 "operable<*> get mismatch bit size"
        );
        Operable* actualB = b.uextIfSizeNotEq(getOperableSlice().getSize());
        auto ret =  new expression(ARITH_MUL,
                                     this,
                                     actualB,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator/( Operable &b) {
        mfAssert(getOperableSlice().getSize() >= b.getOperableSlice().getSize(),
                 "operable</> get mismatch bit size"
        );
        Operable* actualB = b.uextIfSizeNotEq(getOperableSlice().getSize());
        auto ret =  new expression(ARITH_DIV,
                                     this,
                                     actualB,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator%( Operable &b) {
        mfAssert(getOperableSlice().getSize() >= b.getOperableSlice().getSize(),
                 "operable<%> get mismatch bit size"
        );
        Operable* actualB = b.uextIfSizeNotEq(getOperableSlice().getSize());

        auto ret =  new expression(ARITH_DIVR,
                                     this,
                                     actualB,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::extB(int desSize){
        mfAssert(desSize > 0, "desSize must greater than 0");
        mfAssert(getOperableSlice().getSize() == 1, "src extend bit must size must eeq to 1");
        auto ret = new expression(EXTEND_BIT,this, nullptr, desSize);
        return *ret;
    }

    ///// do unsign extend
    Operable& Operable::uext(int desSize){
        mfAssert(desSize > 0, "dessize must greater than 0");
        mfAssert(desSize > getOperableSlice().getSize(), "desSize must greathan original size");
        int oriSize = getOperableSlice().getSize();
        int remainSize = desSize - oriSize;

        auto extendVal = makeOprVal("reSizeTo" +
                               std::to_string(desSize) +
                               "from" + std::to_string(oriSize),
                               remainSize, 0);
        ///// it still work with the system stack
        nest& nextNest = makeNestManReadOnly(true, {this, &extendVal});
        return nextNest;
    }

    Operable* Operable::uextIfSizeNotEq(int desSize){
        if (desSize == getOperableSlice().getSize()){
            return this;
        }
        return &uext(desSize);
    }


    Operable& Operable::sext(int desSize){
        mfAssert(desSize > 0, "dessize must greater than 0");
        mfAssert(desSize > getOperableSlice().getSize(), "desSize must greathan original size");
        int oriSize = getOperableSlice().getSize();
        assert(oriSize >= 1);
        int remainSize = desSize - oriSize;

        auto extendVal = new expression(EXTEND_BIT, this->doSlice({oriSize-1, oriSize}), nullptr, remainSize);
        nest& nextNest = makeNestManReadOnly(true, {this, extendVal});
        return nextNest;
    }




    Operable& Operable::getMatchOperable( ull value) const {
            makeVal(optUserAutoVal, getOperableSlice().getSize(), value);
            /** todo check bit size */
            return optUserAutoVal;
    }

    Operable::operator ull(){
        return v().getVal();

    }
    Operable::operator ValRepBase(){
        return v();

    }

    ValRepBase Operable::v(){
        if(isCacheRepInit){
            return cachedRep;
        }
        mfAssert(getAssignMode() == AM_SIM, "can't retrieve data in model building mode");
        LogicSimEngine* simEngine = getLogicSimEngineFromOpr();
        return simEngine->getProxyRep().slice(getOperableSlice().start, getOperableSlice().stop);
    }

    void Operable::initValRep(const ValRepBase& vrb){
        assert(!isCacheRepInit);
        cachedRep = vrb;
        isCacheRepInit = true;
    }






}