//
// Created by tanawin on 23/7/2024.
//

#include "simValType.h"

#include <model/hwComponent/abstract/operable.h>


namespace kathryn{

    std::string SVT_toUnitType(SIM_VALREP_TYPE_ALL svt){

        if (svt.type == SVT_U64M){
            return "UintX<" + std::to_string(svt.subType) + ">";
        }

        int idx = static_cast<int>(svt.type);
        assert(idx < static_cast<int>(SVT_CNT));
        std::string mapper[SVT_CNT] = {"uint8_t", "uint16_t",
                                       "uint32_t", "uint64_t"};
        return mapper[idx];
    }

    SIM_VALREP_TYPE_ALL getMatchSVT_ALL(Operable* opr){
        assert(opr != nullptr);
        int size = opr->getOperableSlice().getSize();
        assert(size > 0);
        return SIM_VALREP_TYPE_ALL(size);
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


    int getSvtMaxBitSize(SIM_VALREP_TYPE_ALL svt){
        if (svt.type == SVT_U64M){
            return bitSizeOfUll * svt.subType;
        }
        int idx = svt.type;
        return 8* (1 << idx);
    }

    int getArrSize(int size){
        return (size + bitSizeOfUll - 1) / bitSizeOfUll;
    }




}
