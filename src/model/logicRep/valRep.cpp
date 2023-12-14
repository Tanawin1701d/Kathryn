//
// Created by tanawin on 13/12/2566.
//

#include <cassert>
#include <algorithm>
#include "valRep.h"


namespace kathryn{

    ValRep::ValRep(int len): _len(len){
        int alignedSizeInBit = len + (sizeof(ull) << 3) - 1;
        int alignedSizeInByte = alignedSizeInBit >> 3;
        int alighedSizeInUll  = alignedSizeInByte >> 3;

        assert(alighedSizeInUll > 0);
        lenValArr = alighedSizeInUll;
        _val = new ull[lenValArr];
        std::fill(_val, _val + lenValArr, (ull)0);
    }

    ValRep::~ValRep(){
        delete[] _val;
    }

    ValRep::ValRep(const ValRep &rhs) {
        *this = rhs;
    }

    bool ValRep::checkCurrent() const{
        ///// todo we wil optimize it later
        bool isTrue = false;
        for (int i = 0; i < _len; i++){
            int bitInUll =
            isTrue =  isTrue || _val[i]
        }

    }

    ValRep &ValRep::operator=(const ValRep &rhs) {

        if (this == &rhs){
            return *this;
        }
        //////// copy value inside the variable
        _len = rhs._len;
        lenValArr = rhs.lenValArr;
        if (_len == -1){
            _val = nullptr;
        }else{
            assert(_len > 0);
            _val = new ull[lenValArr];
            for (int i = 0; i < lenValArr){
                _val[i] = rhs._val[i];
            }
        }
        return *this;
    }

    /** operator*/
    ValRep ValRep::operator&(const ValRep &rhs) {

        ValRep ret(shinkSize(_len, rhs.getLen()));

        for (int i = 0; i < ret.getLenValArr(); i++){
            ret._val[i] = _val[i] & rhs._val[i];
        }
        return ret;
    }


    ValRep ValRep::operator|(const ValRep& rhs){

        ValRep ret(shinkSize(_len, rhs.getLen()));

        for (int i = 0; i < ret.getLenValArr(); i++){
            ret._val[i] = _val[i] | rhs._val[i];
        }

        return ret;
    }

    ValRep ValRep::operator^(const ValRep& rhs){
        ValRep ret(shinkSize(_len, rhs.getLen()));

        for (int i = 0; i < ret.getLenValArr(); i++){
            ret._val[i] = _val[i] ^ rhs._val[i];
        }

        return ret;
    }

    ValRep ValRep::operator~(){
        ValRep ret(_len);

        for (int i = 0; i < ret.getLenValArr(); i++){
            ret._val[i] = ~_val[i];
        }

        return ret;
    }

    ValRep ValRep::operator << (const ValRep& rhs){
        ValRep ret(shinkSize(_len, rhs.getLen()));

        for (int i = 0; i < ret.getLenValArr(); i++){
            ret._val[i] = _val[i] << rhs._val[i];
        }

        return ret;
    }

    ValRep ValRep::operator >> (const ValRep& rhs){
        ValRep ret(shinkSize(_len, rhs.getLen()));

        for (int i = 0; i < ret.getLenValArr(); i++){
            ret._val[i] = _val[i] >> rhs._val[i];
        }

        return ret;
    }

    ///////// logical expression

    ValRep ValRep::operator && (const ValRep& rhs){
        ValRep ret(shinkSize(_len, rhs.getLen()));

        for (int i = 0; i < ret.getLenValArr(); i++){
            ret._val[i] = _val[i] >> rhs._val[i];
        }

        return ret;
    }


}