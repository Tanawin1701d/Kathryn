//
// Created by tanawin on 6/4/2567.
//

#ifndef KATHRYN_EXTEND_H
#define KATHRYN_EXTEND_H

#include "kathryn.h"

namespace kathryn{


    template<typename T>
    nest& getExtendExpr(T& rawImm, int targetSize, bool unsignExtend) { ///// false is sign extend


        assert(targetSize >= rawImm.getOperableSlice().getSize());

        int extraSize = targetSize - rawImm.getOperableSlice().getSize();
        expression* rawImmExpr = &_make<expression>("extendRaw", rawImm.getOperableSlice().getSize());
        (*rawImmExpr) = rawImm;
        assert(extraSize > 0);
        nest* resultNest;

        if (unsignExtend){
            makeVal(extendBits, extraSize);
            resultNest = &g(extendBits, *rawImmExpr);
            resultNest->setVarName("extend_with_usign");
        }else{
            ////// sign extension

            ///////// get last bit and slice it
            auto* signBit = new expression(1);
            Slice lastBitSlice = rawImm.getOperableSlice();
            lastBitSlice = lastBitSlice.getSubSlice(lastBitSlice.getSize()-1);
            *signBit = *rawImm.doSlice( lastBitSlice );
            //////// make nest from last bit value
            resultNest = &g(makeNestWithSameOneVal(*signBit, extraSize), *rawImmExpr);
            resultNest->setVarName("extend_with_sign");
        }
        return *resultNest;
    }


    nest& getExtendExpr2(Operable& rawImm, int targetSize, bool unsignExtend) { ///// false is sign extend

        assert(targetSize >= rawImm.getOperableSlice().getSize());

        int extraSize = targetSize - rawImm.getOperableSlice().getSize();
        expression* rawImmExpr = &_make<expression>("extendRaw", rawImm.getOperableSlice().getSize());
        (*rawImmExpr) = rawImm;
        assert(extraSize > 0);
        nest* resultNest;

        if (unsignExtend){
            makeVal(extendBits, extraSize);
            resultNest = &g(extendBits, *rawImmExpr);
            resultNest->setVarName("extend_with_usign");
        }else{
            ////// sign extension

            ///////// get last bit and slice it
            auto* signBit = new expression(1);
            Slice lastBitSlice = rawImm.getOperableSlice();
            lastBitSlice = lastBitSlice.getSubSlice(lastBitSlice.getSize()-1);
            *signBit = *rawImm.doSlice( lastBitSlice );
            //////// make nest from last bit value
            resultNest = &g(makeNestWithSameOneVal(*signBit, extraSize), *rawImmExpr);
            resultNest->setVarName("extend_with_sign");
        }
        return *resultNest;
    }


}

#endif //KATHRYN_EXTEND_H
