//
// Created by tanawin on 6/4/2567.
//

#ifndef KATHRYN_EXTEND_H
#define KATHRYN_EXTEND_H

#include "kathryn.h"

namespace kathryn{


    template<typename T>
    Operable& getExtendExpr(T& rawImm, int targetSize, bool unsignExtend) { ///// false is sign extend


        assert(targetSize >= rawImm.getOperableSlice().getSize());
        int   extraSize = targetSize - rawImm.getOperableSlice().getSize();
        Slice lastBitSl =  rawImm.getOperableSlice();
        lastBitSl = {lastBitSl.getSize()-1, lastBitSl.getSize()};

        if (unsignExtend){
            makeVal(zeroExtend, extraSize, 0);
            return g(zeroExtend, rawImm);
        }else{
            return g(rawImm(lastBitSl).extB(extraSize), rawImm);
        }
    }
}

#endif //KATHRYN_EXTEND_H
