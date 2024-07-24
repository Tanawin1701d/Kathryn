//
// Created by tanawin on 23/7/2024.
//

#ifndef SIMVALTYPE_H
#define SIMVALTYPE_H

#include<string>
#include<cassert>

namespace kathryn{


    class Operable;

    constexpr char SIM_VALREP_BASE_NAME [] = "ValR";

    enum SIM_VALREP_TYPE{
        SVT_U8 = 0,
        SVT_U16 = 1,
        SVT_U32 = 2,
        SVT_U64 = 3,
        SVT_U64M = 4,
        SVT_CNT  = 5
    };
    std::string SVT_toUnitType(SIM_VALREP_TYPE svt);
    std::string SVT_toType(SIM_VALREP_TYPE svt);

    SIM_VALREP_TYPE getMatchSVT(Operable* opr);
    SIM_VALREP_TYPE getMatchSVT(int size);

}

#endif //SIMVALTYPE_H
