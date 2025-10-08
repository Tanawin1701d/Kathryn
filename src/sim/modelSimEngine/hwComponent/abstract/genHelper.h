//
// Created by tanawin on 19/7/2024.
//

#ifndef SRCOPRHELPER_H
#define SRCOPRHELPER_H

#include<string>

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/Slice.h"
#include "sim/modelSimEngine/base/simValType.h"


namespace kathryn{
    ValR getSrcOprFromOpr      (Operable* opr1);
    ValR getSlicedSrcOprFromOpr(Operable* opr1);
    ValR getSlicedSrcOprFromOpr(Operable* opr1, SIM_VALREP_TYPE_ALL desField ); ///// the des field that the sliced should be placed
    ValR getSlicedAndShiftSrcOprFromOpr
                               (Operable* opr1, Slice desSlice,
                                SIM_VALREP_TYPE_ALL desField);

    ull createMask(Slice maskSlice);
}

#endif //SRCOPRHELPER_H
