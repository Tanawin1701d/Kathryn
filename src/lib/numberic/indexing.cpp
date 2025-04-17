//
// Created by tanawin on 17/4/2025.
//

#include "indexing.h"

namespace kathryn{




    Operable& cirPtrDis(Operable& bhPtr, Operable& frPtr){
        int bhs = bhPtr.getOperableSlice().getSize();
        int frs = frPtr.getOperableSlice().getSize();
        mfAssert(bhs == frs,
                 "theSize of cirPtr is not equal bhPtr got " +
                 std::to_string(bhs) + " but frPtr got " +
                 std::to_string(frs)
                 );
        mfAssert(bhs <64, "cirPtrDis not support >= 64 bit size");

        mWire(cirDis, bhPtr.getOperableSlice().getSize());
        mVal(cirDisZero, bhs, 0);

        zif(bhPtr <= frPtr){
            cirDis = frPtr - bhPtr;
        }zelse{
            cirDis = cirDisZero - (bhPtr - frPtr);
        }

        return cirDis;

    }

}