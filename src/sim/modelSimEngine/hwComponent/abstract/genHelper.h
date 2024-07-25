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
    ValR getSrcOprFromOpr      (Operable* opr);
    ValR getSlicedSrcOprFromOpr(Operable* opr);
    ValR getSlicedSrcOprFromOpr(Operable* opr, SIM_VALREP_TYPE desField ); ///// the des field that the sliced should be placed
    ValR getSlicedAndShiftSrcOprFromOpr
                               (Operable* opr, Slice desSlice,
                                SIM_VALREP_TYPE desField);

    ull createMask(Slice maskSlice);
}

#endif //SRCOPRHELPER_H
