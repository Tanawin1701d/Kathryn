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
        _len = -1;
        lenValArr = -1;
        _val =
    }


}