//
// Created by tanawin on 23/7/2024.
//

#include "simValType.h"

#include <model/hwComponent/abstract/operable.h>


namespace kathryn{

    std::string SVT_toUnitType(SIM_VALREP_TYPE svt){

        int idx = static_cast<int>(svt);

        assert(idx < static_cast<int>(SVT_CNT));
        std::string mapper[SVT_CNT] = {"uint8_t", "uint16_t",
                                       "uint32_t", "uint64_t"};
        return mapper[idx];
    }

    std::string SVT_toType(SIM_VALREP_TYPE svt){

        std::string ret;

        ret += SIM_VALREP_BASE_NAME;
        ret += "<";
        ret += SVT_toUnitType(svt);
        ret += ">";

        return ret;

    }

    SIM_VALREP_TYPE getMatchSVT(Operable* opr){
        assert(opr != nullptr);
        int size = opr->getOperableSlice().getSize();

        return getMatchSVT(size);
    }

    SIM_VALREP_TYPE getMatchSVT(int size){
        if (size <= 8){
            return SVT_U8;
        }
        if (size <= 16){
            return SVT_U16;
        }
        if (size <= 32){
            return SVT_U32;
        }
        if (size <= 64){
            return SVT_U64;
        }

        return SVT_U64M;
    }




}
