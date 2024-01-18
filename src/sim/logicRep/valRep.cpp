//
// Created by tanawin on 13/12/2566.
//

#include <cassert>
#include <algorithm>
#include "valRep.h"


namespace kathryn{


    ValRep::ValRep(int len) {

        assert(len > 0);
        _len = len;
        _valSize = (_len + bitSizeOfUll - 1 )/ bitSizeOfUll; //// ceiling of _len/64
        _val = new ull[_valSize]();
        std::fill(_val, _val + _valSize, 0);

    }

    ValRep::~ValRep() {
        delete[] _val;
    }

    ValRep::ValRep(const ValRep &rhs) {

        _len = rhs._len;
        _valSize = rhs._valSize;
        std::copy(rhs._val, rhs._val + _valSize, _val);

    }


    ValRep ValRep::getZeroExtend(int targetSize) {

        assert(targetSize >= _len);
        assert(targetSize > 0);

        ValRep preRet(targetSize);
        /** copy to destination*/
        for (int cpIdx = 0; cpIdx < _valSize; cpIdx++){
            preRet._val[cpIdx] = _val[cpIdx];
        }
        /** zero extension*/
        for (int fillIdx = _valSize; fillIdx < preRet._valSize; fillIdx++){
            preRet._val[fillIdx] = 0;
        }

        return preRet;

    }

    ////////////// required equal bit operator

    ValRep ValRep::bwOperator(const ValRep &rhs,
                              const std::function<ull(ull, ull)>& operation) {

        /** check valid size*/
        assert(checkEqualBit(rhs));
        assert(_len > 0);
        /**build new variable*/
        ValRep preRet(_len);
        for (int valIdx = 0; valIdx < _valSize; valIdx++){
            preRet._val[valIdx] = operation(_val[valIdx], rhs._val[valIdx]);
        }
        return preRet;

    }

    ValRep ValRep::eqOperator(const ValRep &rhs, bool checkEq) {
        /** check valid size*/
        assert(checkEqualBit(rhs));
        assert(_len > 0);
        /**build new variable*/
        ValRep preRet(1);
        bool result = true;
        for (int valIdx = 0; valIdx < _valSize; valIdx++){
            result =  result && (_val[valIdx] == rhs._val[valIdx]);
        }
        if (checkEq){
            preRet._val[0] = result ? 1 : 0;
        }else{
            preRet._val[0] = result ? 0 : 1;
        }

        return preRet;
    }

    bool ValRep::getLogicalValue() const{
        bool result = false;
        for (int valIdx = 0; valIdx < _valSize; valIdx++){
            result = result || _val[valIdx];
        }
        return result;
    }


    ValRep ValRep::logicalOperator(const ValRep &rhs,
                                   const std::function<bool(bool, bool)>& operation
                                   )const
    {

        assert(_len > 0 && rhs._len > 0);
        ValRep preRet(1);
        bool aLogic = getLogicalValue();
        bool bLogic = rhs.getLogicalValue();
        preRet._val[0] = operation(aLogic, bLogic) ? 1 : 0;

        return preRet;

    }

    ValRep ValRep::cmpOperator(const ValRep &rhs,
                               const std::function<bool(ull* a, ull* b, int sz)>& operation) {

        /** extend value to achieve value*/
        int maxLen = std::max(getLen(), rhs.getLen());
        ValRep SrcA = getZeroExtend(maxLen);
        ValRep SrcB = getZeroExtend(maxLen);

        ValRep preRet(1);
        preRet._val[0] = operation(SrcA._val, SrcB._val, maxLen) ? 1 : 0;
        return preRet;

    }

    ValRep &ValRep::operator=(const ValRep &rhs) {

        if (&rhs == this){
            return *this;
        }

        _len = rhs._len;
        _valSize = rhs._valSize;
        if (_valSize > 0){
            _val = new ull[_valSize];
            std::copy(rhs._val, rhs._val + _valSize, _val);
        }else{
            _val = nullptr;
        }

        return *this;
    }


    /////////////////////////////////////////////////////////////
    //////////// bitwise operator ///////////////////////////////
    /////////////////////////////////////////////////////////////



    ValRep ValRep::operator&(const ValRep &rhs) {
        return bwOperator(rhs, [](ull a, ull b) -> ull
                                {return a & b;});
    }

    ValRep ValRep::operator|(const ValRep &rhs) {
        return bwOperator(rhs, [](ull a, ull b) -> ull
                                {return a | b;});
    }

    ValRep ValRep::operator^(const ValRep &rhs) {
        return bwOperator(rhs, [](ull a, ull b) -> ull
                                {return a ^ b;});
    }
    ////////////  inverse operand
    ValRep ValRep::operator~() {
        /** check valid size*/
        assert(_len > 0);
        /**build new variable*/
        ValRep preRet(_len);
        for (int valIdx = 0; valIdx < _valSize; valIdx++){
            preRet._val[valIdx] = ~_val[valIdx];
        }
        return preRet;
    }

    /////////////////////////////////////////////////////////////
    //////////// equal operator /////////////////////////////////
    /////////////////////////////////////////////////////////////

    ValRep ValRep::operator==(const ValRep &rhs) {
        return eqOperator(rhs, true);
    }

    ValRep ValRep::operator!=(const ValRep &rhs) {
        return eqOperator(rhs, false);
    }


    ////////////////////////////////////////////////////////////
    /////////// logical operator ///////////////////////////////
    ////////////////////////////////////////////////////////////

    ValRep ValRep::operator&&(const ValRep& rhs) const {
        return logicalOperator(rhs, []( bool a, bool b)-> bool {
                                    return a && b;
                                });
    }

    ValRep ValRep::operator||(const ValRep& rhs) const {
        return logicalOperator(rhs, []( bool a, bool b)-> bool {
            return a || b;
        });
    }

    ValRep ValRep::operator!()const
    {

        assert(_len > 0);
        ValRep preRet(1);
        bool aLogic = getLogicalValue();
        preRet._val[0] = aLogic ? 0 : 1; ////// inverse logic
        //////// we must explicitly tell the value due to prevent compiler confusing

        return preRet;

    }


    ////////////////////////////////////////////////////////////
    /////////// compare operator ///////////////////////////////
    ////////////////////////////////////////////////////////////

    ValRep ValRep::operator<(const ValRep &rhs) {

        return cmpOperator(rhs,[](const ull* valA, const ull* valB, int valSize) -> bool
                           {
                                assert(valSize >= 1);
                                for (int cmpIdx = valSize-1; cmpIdx >= 0; cmpIdx--){
                                    if (valA[cmpIdx] < valB[cmpIdx]){
                                        return true;
                                    }else if (valA[cmpIdx] == valB[cmpIdx]){
                                        continue;
                                    }else{
                                        return false;
                                    }
                                }
                                return false;
                           });

    }

    ValRep ValRep::operator<=(const ValRep &rhs) {
        return cmpOperator(rhs,[](const ull* valA, const ull* valB, int valSize) -> bool
        {
            assert(valSize >= 1);
            for (int cmpIdx = valSize-1; cmpIdx >= 0; cmpIdx--){
                if (valA[cmpIdx] < valB[cmpIdx]){
                    return true;
                }else if (valA[cmpIdx] == valB[cmpIdx]){
                    continue;
                }else{
                    return false;
                }
            }
            return true;
        });
    }

    ValRep ValRep::operator>(const ValRep &rhs) {
        return cmpOperator(rhs,[](const ull* valA, const ull* valB, int valSize) -> bool
        {
            assert(valSize >= 1);
            for (int cmpIdx = valSize-1; cmpIdx >= 0; cmpIdx--){
                if (valA[cmpIdx] > valB[cmpIdx]){
                    return true;
                }else if (valA[cmpIdx] == valB[cmpIdx]){
                    continue;
                }else{
                    return false;
                }
            }
            return false;
        });
    }

    ValRep ValRep::operator>=(const ValRep &rhs) {
        return cmpOperator(rhs,[](const ull* valA, const ull* valB, int valSize) -> bool
        {
            assert(valSize >= 1);
            for (int cmpIdx = valSize-1; cmpIdx >= 0; cmpIdx--){
                if (valA[cmpIdx] > valB[cmpIdx]){
                    return true;
                }else if (valA[cmpIdx] == valB[cmpIdx]){
                    continue;
                }else{
                    return false;
                }
            }
            return true;
        });
    }

    ////////////////////////////////////////////////////////////
    /////////// arithmetic operator ////////////////////////////
    ////////////////////////////////////////////////////////////

    ValRep ValRep::operator+(const ValRep &rhs) {
        /** extend value to achieve value*/
        int maxLen = std::max(getLen(), rhs.getLen());
        ValRep SrcA = getZeroExtend(maxLen);
        ValRep SrcB = getZeroExtend(maxLen);

        assert(maxLen >= 1);
        ValRep preRet(maxLen + 1);

        /***addition*/
        bool overFlow = false;
        for (int valIdx = 0; valIdx < SrcA._valSize; valIdx++){
            preRet._val[valIdx] = SrcA._val[valIdx] + SrcB._val[valIdx] + overFlow;

            overFlow =  preRet._val[valIdx] < SrcA._val[valIdx] ||
                        preRet._val[valIdx] < SrcB._val[valIdx];

        }

        if (overFlow){
            preRet._val[SrcA._valSize] = 1;
        }

        return preRet;
    }

    ValRep ValRep::operator-(const ValRep &rhs) {
        int maxLen = std::max(getLen(), rhs.getLen());
        ValRep srcA = getZeroExtend(maxLen);
        ValRep srcB = getZeroExtend(maxLen);

        assert(maxLen >= 1);
        ValRep preRet(maxLen);

        /***addition with plus 1*/
        srcB = ~srcB;
        bool overFlow = true; //// we must set to true due to minus number
        for (int valIdx = 0; valIdx < srcA._valSize; valIdx++){
            preRet._val[valIdx] = srcA._val[valIdx] + srcB._val[valIdx] + overFlow;
        }

        return preRet;
    }

    ValRep ValRep::operator<<(const ValRep &rhs) {

        /** we don't let user shift than 2^64 */
        ValRep preRet(_len);
        assert(rhs._valSize == 1);
        int shiftEntireVal = (int)rhs._val[0] / bitSizeOfUll;
        int shiftminorVal  = (int)rhs._val[0] % bitSizeOfUll;

        for (int valIdx = _valSize-1; valIdx >= 0; valIdx--){
            if ((valIdx - shiftEntireVal) >= 0){
                /*** major shift to align 64 bit per array block*/
                preRet._val[valIdx] = _val[valIdx - shiftEntireVal];
                /** do minor shift*/
                preRet._val[valIdx] = preRet._val[valIdx] << shiftminorVal;
                /*** if there are minor value to shift*/
                if ((valIdx - shiftEntireVal - 1) >= 0){
                    preRet._val[valIdx] |= (_val[valIdx - shiftEntireVal - 1] >> (bitSizeOfUll - shiftminorVal));
                }
            }else{
                preRet._val[valIdx] = 0;
            }
        }

        return preRet;

    }

    ValRep ValRep::operator>>(const ValRep &rhs) {
        /** we don't let user shift than 2^64 */
        ValRep preRet(_len);
        assert(rhs._valSize == 1);
        int shiftEntireVal = (int)rhs._val[0] / bitSizeOfUll;
        int shiftminorVal  = (int)rhs._val[0] % bitSizeOfUll;

        for (int valIdx = 0; valIdx < _valSize; valIdx++){
            if ((valIdx + shiftEntireVal) < _valSize){
                    /*** major shift to align 64 bit per array block*/
                    preRet._val[valIdx] = _val[valIdx + shiftEntireVal];
                    /** do minor shift this is ensured that less than 64 bit*/
                    preRet._val[valIdx] = preRet._val[valIdx] >> shiftminorVal;
                    if ((valIdx + shiftEntireVal + 1) < _valSize){
                        preRet._val[valIdx] |= (_val[valIdx + shiftEntireVal + 1] << (bitSizeOfUll - shiftminorVal));
                    }
            }else{
                preRet._val[valIdx] = 0;
            }
        }

        return preRet;
    }


}