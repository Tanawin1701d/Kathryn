//
// Created by tanawin on 23/7/2024.
//

#include "simValType.h"

#include <model/hwComponent/abstract/operable.h>


namespace kathryn{

    SIM_VALREP_TYPE_ALL getMatchSVT_ALL(Operable* opr1){
        assert(opr1 != nullptr);
        int size = opr1->getOperableSlice().getSize();
        assert(size > 0);
        return SIM_VALREP_TYPE_ALL(size);
    }

    SIM_VALREP_TYPE getMatchSVT(int size){
        if (size <= 8){return SVT_U8;}
        if (size <= 16){return SVT_U16;}
        if (size <= 32){return SVT_U32;}
        if (size <= 64){return SVT_U64;}
        return SVT_U64M;
    }

    SIM_VALREP_TYPE_INT getMatchSVTI(int size){
        if (size <= 8){return SVTI_8;}
        if (size <= 16){return SVTI_16;}
        if (size <= 32){return SVTI_32;}
        if (size <= 64){return SVTI_64;}
        return SVTI_64M;

    }

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

    std::string SVTI_toUnitType(SIM_VALREP_TYPE_I_ALL svt){
        if (static_cast<int>(svt.type) == static_cast<int>(SVT_U64M)){
            mfAssert(false, "for now > 64 bit value can't support sign input");
        }
        int idx = static_cast<int>(svt.type);
        assert(idx < static_cast<int>(SVT_CNT));
        std::string mapper[SVT_CNT] = {"int8_t", "int16_t", "int32_t", "int64_t"};
        return mapper[idx];
    }

    std::string SVT_toUnitRefType(SIM_VALREP_TYPE_ALL svt){
        return SVT_toUnitType(svt) + "&";
    }

    std::string SVT_toUnitPtrType(SIM_VALREP_TYPE_ALL svt){
        return SVT_toUnitType(svt) + "*";
    }

    std::string SVTI_toUnitRefType(SIM_VALREP_TYPE_I_ALL svt){
        return SVTI_toUnitType(svt) + "&";
    }

    std::string SVTI_toUnitPtrType(SIM_VALREP_TYPE_I_ALL svt){
        return SVTI_toUnitType(svt) + "*";
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

    SIM_VALREP_TYPE_I_ALL cvtValRepType(SIM_VALREP_TYPE_ALL svt){
        int subTypeClone = svt.subType;
        SIM_VALREP_TYPE_INT convertedType = static_cast<SIM_VALREP_TYPE_INT>(svt.type);

        SIM_VALREP_TYPE_I_ALL ret(convertedType, subTypeClone);
        return ret;
    }





}
