//
// Created by tanawin on 19/7/2024.
//

#ifndef SRCOPRHELPER_H
#define SRCOPRHELPER_H

#include<string>

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/Slice.h"


namespace kathryn{
    std::string getSrcOprFromOpr(Operable* opr);
    std::string getSlicedSrcOprFromOpr(Operable* opr);
    std::string getSlicedAndShiftSrcOprFromOpr(Operable* opr, Slice desSlice);

    ull createMask(Slice maskSlice);
}

#endif //SRCOPRHELPER_H
