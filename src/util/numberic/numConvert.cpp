//
// Created by tanawin on 25/1/2567.
//

#include "numConvert.h"

namespace kathryn{

    ValRep genBiConValRep(bool val, int bitSize){
        ValRep preRet(bitSize);
        if (!val){return preRet;}

        ull* preRetArr = preRet.getVal();
        for (int i = 0; i < preRet.getValArrSize(); i++){
            preRetArr[i] = (ull)(-1);
        }
        preRet.fillZeroToValrep(preRet.getLen());
        return preRet;
    }

}