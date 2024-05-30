//
// Created by tanawin on 7/5/2567.
//

#include <iostream>
#include"valRep.h"

namespace kathryn {

#ifdef NOTEXCEED64

    ValRep::ValRep(int len):
    _len(len),
    _valSize(1),
    _val(0){
        assert(len <= bitSizeOfUll);
        assert(_len > 0);
    }

    ValRep::~ValRep() = default;

    ValRep::ValRep(const ValRep &rhs) {
        operator=(rhs);
    }

    ull* ValRep::getValPtr() {
        return &_val;
    }

    ull ValRep::getVal64() const {
        return _val;
    }

    ValRep::operator ull() {
        return _val;
    }

    ValRep ValRep::getZeroExtend(int targetSize) {
        assert(targetSize >= _len);
        assert(targetSize <=  bitSizeOfUll);
        return *this;
    }

    ValRep ValRep::shink(int targetSize) {
        assert( (targetSize > 0) && (targetSize <= getLen()));
        ValRep preRet(*this);
        preRet._len = targetSize;

        preRet.fillZeroToValrep(preRet.getLen());

        return preRet;
    }

    ValRep ValRep::bwOperator(const ValRep &rhs, const std::function<ull(ull, ull)> &operation) {
        assert(false);
    }

    ValRep ValRep::logicalOperator(const ValRep &rhs, const std::function<bool(bool, bool)> &operation) const {
        assert(false);
    }

    ValRep ValRep::cmpOperator(ValRep &rhs, const std::function<bool(ull *, ull *, int)> &operation) {
        assert(false);
    }

    ValRep ValRep::eqOperator(const ValRep &rhs, bool checkEq) {
        assert(false);
    }

    bool ValRep::getLogicalValue() const {
        return _val;
    }

    void ValRep::update(ValRep& srcVal){
        assert(srcVal.getLen() == getLen());
        _val = srcVal._val;
    }

    ValRep ValRep::slice(Slice sl) {
        assert(sl.stop <= getLen());
        assert(sl.checkValidSlice());

        ValRep cpy(*this);
        cpy._val = cpy._val >> sl.start;
        ValRep shinkedVal = cpy.shink(sl.getSize());

        return shinkedVal;
    }

    void ValRep::updateOnSlice(ValRep srcVal, Slice desSl) {
        assert(desSl.getSize() == srcVal.getLen());
        assert(desSl.stop <= getLen());
        assert(desSl.checkValidSlice());

        ////// create new value
        ull zeroMask  = getZeroMask(desSl.start, desSl.stop);
        ull newAddVal = srcVal._val;
        newAddVal     = newAddVal << desSl.start;
        newAddVal     = newAddVal & (~zeroMask);

        _val = _val & zeroMask; /// clear old vavlue
        _val = _val | newAddVal; //// add new value
    }

    ull ValRep::getZeroMask(int startIdx, int stopIdx) const {
        ///////[startIdx, stopIdx)
        /**check integrity**/
        assert(startIdx < stopIdx);
        assert(startIdx >= 0);
        assert(stopIdx <= bitSizeOfUll);
        int amtConsiderBit = stopIdx - startIdx;
        ull mask;
        if (amtConsiderBit == bitSizeOfUll){
            mask = (ull)(-1); //// corner case shift over varsize will be not shifted in c++
            assert( (mask + 1) == 0);
        }else{
            mask =  (((ull)1) << amtConsiderBit) - 1;
        }
        ///////////(5 bit) 100000  - 1 = 011111
        mask = mask << startIdx; //// shift to right position first
        mask = ~mask; ///// we need zero bit to mask the consider region
        return mask;
    }

    void ValRep::fillZeroToValrep(int startBit, int stopBit) {

        assert( (startBit >= 0) && (startBit < stopBit));
        assert(stopBit <= bitSizeOfUll);

        ull zeroMask = getZeroMask(startBit ,stopBit);
        _val = _val & zeroMask;

    }

    void ValRep::fillZeroToValrep(int startBit) {

        fillZeroToValrep(startBit, bitSizeOfUll);

    }


    ValRep& ValRep::operator=(const ValRep &rhs) {
        _len     = rhs._len;
        _valSize = rhs._valSize;
        _val     = rhs._val;
        return *this;
    }

    std::string ValRep::getBiStr() {

        // std::string preRet;
        // preRet.resize(getLen());
        // ull cpyVal = _val;
        // for (int i = 0; i < getLen(); i++) {
        //
        //     preRet[getLen()-i-1] += '0' + (cpyVal&1);
        //     cpyVal >>= 1;
        //
        // }

        std::string preRet ;
        assert(getLen() > 0);
        for (int i = getValArrSize()-1; i >= 0; i--){
            std::bitset<bitSizeOfUll> binaryRepresentation(_val);
            preRet += binaryRepresentation.to_string();
        }
        return preRet.substr(preRet.size() - getLen(), getLen());

        /////// std::cout << preRet << std::endl;
        return preRet;
    }


    ValRep ValRep::operator&(const ValRep &rhs) {
        assert(checkEqualBit(rhs));
        ValRep preRet = *this;
        preRet._val &= rhs._val;
        return preRet;
    }

    ValRep ValRep::operator|(const ValRep &rhs) {
        assert(checkEqualBit(rhs));
        ValRep preRet = *this;
        preRet._val |= rhs._val;
        return preRet;
    }

    ValRep ValRep::operator^(const ValRep &rhs) {
        assert(checkEqualBit(rhs));
        ValRep preRet = *this;
        preRet._val ^= rhs._val;
        return preRet;
    }

    ValRep ValRep::operator==(const ValRep &rhs) {
        assert(checkEqualBit(rhs));
        ValRep preRet(1);
        preRet._val = (_val == rhs._val);
        return preRet;
    }

    ValRep ValRep::operator!=(const ValRep &rhs) {
        assert(checkEqualBit(rhs));
        ValRep preRet(1);
        preRet._val = (_val != rhs._val);
        return preRet;
    }

    ValRep ValRep::operator~() {
        ////std::cout << "or.... " << _val << std::endl;
        ValRep preRet = *this;
        preRet._val = ~preRet._val;
        fillZeroToValrep(preRet.getLen());
        ////std::cout << "or.... " << preRet._val << std::endl;
        return preRet;
    }

    ValRep ValRep::operator&&(const ValRep &rhs) const {
        assert(checkEqualBit(rhs));
        ValRep preRet(1);
        preRet._val = (_val && rhs._val);
        return preRet;
    }

    ValRep ValRep::operator||(const ValRep &rhs) const {
        assert(checkEqualBit(rhs));
        ValRep preRet(1);
        preRet._val = (_val || rhs._val);
        return preRet;
    }

    ValRep ValRep::operator!() const {
        ValRep preRet(1);
        preRet._val = (!_val);
        return preRet;
    }

    ValRep ValRep::operator<(ValRep &rhs) {
        //////assert(checkEqualBit(rhs));
        ValRep preRet(1);
        preRet._val = (_val < rhs._val);
        return preRet;
    }

    ValRep ValRep::operator<=(ValRep &rhs) {
        /////assert(checkEqualBit(rhs));
        ValRep preRet(1);
        preRet._val = (_val <= rhs._val);
        return preRet;
    }

    ValRep ValRep::operator>(ValRep &rhs) {
        ////assert(checkEqualBit(rhs));
        ValRep preRet(1);
        preRet._val = (_val > rhs._val);
        ///assert(preRet._val == 0);
        return preRet;
    }

    ValRep ValRep::operator>=(ValRep &rhs) {
        /////assert(checkEqualBit(rhs));
        ValRep preRet(1);
        preRet._val = (_val >= rhs._val);
        return preRet;
    }

    ValRep ValRep::operator+(ValRep rhs) {
        int maxBit = std::max(getLen(), rhs.getLen());
        ValRep preRet(maxBit);
        preRet._val = _val + rhs._val;
        preRet.fillZeroToValrep(preRet.getLen());
        return preRet;
    }

    ValRep ValRep::operator-(ValRep rhs) {
        int maxLen = std::max(getLen(), rhs.getLen());
        ValRep srcA = getZeroExtend(maxLen);
        ValRep srcB = rhs.getZeroExtend(maxLen);

        assert(maxLen >= 1);
        ValRep preRet(maxLen);

        /***addition with plus 1*/
        srcB = ~srcB;
        ValRep overFlow = ValRep(1);
        overFlow._val = 1;

        preRet = srcA + srcB + overFlow;
        /** prevent overflow bit pollute*/
        preRet.fillZeroToValrep(preRet.getLen());
        //////////////////////////////////////////////////////////////

        return preRet;
    }

    ValRep ValRep::operator<<(const ValRep &rhs) {

        ValRep preRet(*this);
        if (rhs._val < bitSizeOfUll){
            preRet._val <<= rhs._val;
        }else{
            preRet._val = 0;
        }

        preRet.fillZeroToValrep(preRet.getLen());
        return preRet;
    }

    ValRep ValRep::operator<<(int rhs) {
        ValRep shiftIdent(bitSizeOfUll);
        shiftIdent._val = (ull)rhs;
        return operator<<(shiftIdent);
    }

    ValRep ValRep::operator>>(const ValRep &rhs) {
        ValRep preRet(*this);
        if (rhs._val < bitSizeOfUll){
            preRet._val >>= rhs._val;
        }else{
            preRet._val = 0;
        }

        preRet.fillZeroToValrep(preRet.getLen());
        return preRet;
    }

    ValRep ValRep::operator>>(int rhs) {
        ValRep shiftIdent(bitSizeOfUll);
        shiftIdent._val = (ull)rhs;
        return operator>>(shiftIdent);
    }


}



#endif