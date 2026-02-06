//
// Created by tanawin on 6/2/26.
//

#include "math.h"

namespace kathryn{

    Operable& sqrtInt(Operable& x){
        int bs = x.getOperableSlice().getSize();
        assert(bs > 0);
        mReg(xc, bs);
        mReg(y, bs);
        seq{
            par{ xc <<= x; y<<= x;}
            Operable& yNext = (y + xc/y) >> 1;
            cdowhile(yNext < y){
                y <<= yNext;
            }
        }
        return y;
    }
}