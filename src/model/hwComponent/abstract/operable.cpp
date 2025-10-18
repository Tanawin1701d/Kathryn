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

    /////// blA = balanced A
    /////// blB = balanced B

/** bitwise operators*/
    expression& Operable::operator&( Operable &b) {
        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
               "operable<&> get mismatch bit size");

        auto [blA, blB] = uextToBalanceSize(*this, b);
        int  desSize    = balanceSize(*this, b);
        auto ret =  new expression(BITWISE_AND,
                                   &blA,
                                   &blB,
                                   desSize);
        return *ret;
    }

    expression& Operable::operator|(Operable &b) {
        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
               "operable<|> get mismatch bit size");

        auto [blA, blB] = uextToBalanceSize(*this, b);
        int  desSize    = balanceSize(*this, b);
        auto ret =  new expression(BITWISE_OR,
                                   &blA,
                                   &blB,
                                   desSize);

        return *ret;
    }

    expression& Operable::operator^( Operable &b) {
        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
               "operable<^> get mismatch bit size"
        );

        auto [blA, blB] = uextToBalanceSize(*this, b);
        int  desSize    = balanceSize(*this, b);

        auto ret =  new expression(BITWISE_XOR,
                                    &blA,
                                    &blB,
                                    desSize);

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

        mfWarn(b.getOperableSlice().getSize() <= 6,  "warning there is shift bit that greater than 64");
        auto ret =  new expression(BITWISE_SHL,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator>>( Operable &b) {

        mfWarn(b.getOperableSlice().getSize() <= 6,  "warning there is shift bit that greater than 64");
        auto ret =  new expression(BITWISE_SHR,
                                     this,
                                     &b,
                                     getOperableSlice().getSize());

        return *ret;
    }


    /** logical operators*/

    expression& Operable::operator&&( Operable &b) {
        mfWarn(getOperableSlice().getSize() == 1 &&
               b.getOperableSlice().getSize() == 1,
               "operable && got size expect to have size equal to 1");
        auto ret =  new expression(LOGICAL_AND,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator||( Operable &b) {
        mfWarn(getOperableSlice().getSize() == 1 &&
               b.getOperableSlice().getSize() == 1,
               "operable || got size expect to have size equal to 1");
        auto ret =  new expression(LOGICAL_OR,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!() {
        mfWarn(getOperableSlice().getSize() == 1,
               "operable ! got size expect to have size equal to 1");
        auto ret =  new expression(LOGICAL_NOT,
                                     this,
                                     nullptr,
                                     LOGICAL_SIZE);

        return *ret;
    }

    /** relational operator*/

    expression& Operable::operator==( Operable &b) {

        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
               "operable<==> get mismatch bit size");

        auto [blA, blB] = uextToBalanceSize(*this, b);

        auto ret =  new expression(RELATION_EQ,
                                     &blA,
                                     &blB,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!=( Operable &b) {

        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
       "operable<!=> get mismatch bit size");

        auto [blA, blB] = uextToBalanceSize(*this, b);

        auto ret =  new expression(RELATION_NEQ,
                                     &blA,
                                     &blB,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<( Operable &b) {

        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
       "operable< < > get mismatch bit size");

        auto [blA, blB] = uextToBalanceSize(*this, b);

        auto ret =  new expression(RELATION_LE,
                                    &blA,
                                    &blB,
                                    LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<=( Operable &b) {

        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
       "operable< <= > get mismatch bit size");

        auto [blA, blB] = uextToBalanceSize(*this, b);

        auto ret =  new expression(RELATION_LEQ,
                                     &blA,
                                     &blB,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>( Operable &b) {

        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
       "operable< > > get mismatch bit size");

        auto [blA, blB] = uextToBalanceSize(*this, b);

        auto ret =  new expression(RELATION_GE,
                                     &blA,
                                     &blB,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>=( Operable &b) {

        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
       "operable< >= > get mismatch bit size");

        auto [blA, blB] = uextToBalanceSize(*this, b);

        auto ret =  new expression(RELATION_GEQ,
                                     &blA,
                                     &blB,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::slt(Operable& b){

        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
       "operable< slt(sign less than) > get mismatch bit size");

        auto [blA, blB] = sextToBalanceSize(*this, b);

        auto ret = new expression(RELATION_SLT,
                                    &blA,
                                    &blB,
                                    LOGICAL_SIZE);
        return *ret;
    }

    expression& Operable::sgt(Operable& b){

        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
       "operable< slt(sign greater than) > get mismatch bit size");

        auto [blA, blB] = sextToBalanceSize(*this, b);

        auto ret = new expression(RELATION_SGT,
                                    &blA,
                                    &blB,
                                    LOGICAL_SIZE);
        return *ret;
    }



    /** arithmetic operators*/

    expression& Operable::operator+( Operable &b) {
        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
                 "operable<+> get mismatch bit size"
        );

        auto [blA, blB] = uextToBalanceSize(*this, b);
        auto ret =  new expression(ARITH_PLUS,
                                     &blA,
                                     &blB,
                                     getOperableSlice().getSize());
        /** size + 1 because we provide carry for exprMetas*/

        return *ret;
    }

    expression& Operable::operator-( Operable &b) {
        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
                 "operable<-> get mismatch bit size"
        );
        auto [blA, blB] = uextToBalanceSize(*this, b);

        auto ret =  new expression(ARITH_MINUS,
                                     &blA,
                                     &blB,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator*( Operable &b) {
        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
                 "operable<*> get mismatch bit size"
        );

        auto [blA, blB] = uextToBalanceSize(*this, b);
        auto ret =  new expression(ARITH_MUL,
                                     &blA,
                                     &blB,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator/( Operable &b) {
        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
                 "operable</> get mismatch bit size"
        );

        auto [blA, blB] = uextToBalanceSize(*this, b);
        auto ret =  new expression(ARITH_DIV,
                                     &blA,
                                     &blB,
                                     getOperableSlice().getSize());

        return *ret;
    }

    expression& Operable::operator%( Operable &b) {
        mfWarn(getOperableSlice().getSize() == b.getOperableSlice().getSize(),
                 "operable<%> get mismatch bit size"
        );

        auto [blA, blB] = uextToBalanceSize(*this, b);
        auto ret =  new expression(ARITH_DIVR,
                                     &blA,
                                     &blB,
                                     getOperableSlice().getSize());

        return *ret;
    }


    int Operable::balanceSize(Operable& a, Operable& b) {
        int aSize = a.getOperableSlice().getSize();
        int bSize = b.getOperableSlice().getSize();
        assert((aSize > 0) && (bSize > 0));
        return std::max(aSize, bSize);
    }

    expression& Operable::extB(int desSize){
        mfAssert(desSize > 0, "desSize must greater than 0");
        mfAssert(getOperableSlice().getSize() == 1, "src extend bit must size must eeq to 1");
        auto ret = new expression(EXTEND_BIT,this, nullptr, desSize);
        return *ret;
    }
    /////////////////////////////////
    ///// do unsign extend //////////
    /////////////////////////////////
    Operable& Operable::uext(int desSize){
        mfAssert(desSize > 0, "dessize must greater than 0");
        mfAssert(desSize > getOperableSlice().getSize(), "desSize must greathan original size");
        int oriSize = getOperableSlice().getSize();
        int remainSize = desSize - oriSize;

        auto& extendVal = makeOprVal("reSizeTo" +
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

    std::pair<Operable&, Operable&> Operable::uextToBalanceSize(Operable& a,
                                                                Operable& b){
        /////////// check size
        int desSize = balanceSize(a, b);
        /////////// upgrade size
        Operable* upgradedA = uextIfSizeNotEq(desSize);
        Operable* upgradedB = b.uextIfSizeNotEq(desSize);

        return {*upgradedA, *upgradedB};
    }

    /////////////////////////////////
    ///// do sign extend   //////////
    /////////////////////////////////

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

    Operable* Operable::sextIfSizeNotEq(int desSize){
        if (desSize == getOperableSlice().getSize()){
            return this;
        }
        return &sext(desSize);
    }

    std::pair<Operable&, Operable&>
    Operable::sextToBalanceSize(Operable& a, Operable& b){
        /////////// check size
        int desSize = balanceSize(a, b);
        /////////// upgrade size
        Operable* upgradedA = sextIfSizeNotEq(desSize);
        Operable* upgradedB = b.sextIfSizeNotEq(desSize);

        return {*upgradedA, *upgradedB};
    }


    Operable& Operable::sl(int start, int stop){
        return *doSlice({start, stop});
    }

    Operable& Operable::sl(int start){
        return *doSlice({start, start+1});
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