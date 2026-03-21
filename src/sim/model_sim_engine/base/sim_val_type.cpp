//
// Created by tanawin on 23/7/2024.
//

#include "sim_val_type.h"

#include "model/hw_component/abstract/operable.h"


namespace kathryn{

    SIM_VALREP_TYPE_ALL getMatchSVT_ALL(Operable* opr1){
        assert(opr1 != nullptr);
        int size = opr1->get_operable_slice().get_size();
        assert(size > 0);
        return SIM_VALREP_TYPE_ALL(size);
    }

    SIM_VALREP_TYPE get_match_svt(int size){
        if (size <= 8){return SVT_U8;}
        if (size <= 16){return SVT_U16;}
        if (size <= 32){return SVT_U32;}
        if (size <= 64){return SVT_U64;}
        return SVT_U64M;
    }

    SIM_VALREP_TYPE_INT get_match_svti(int size){
        if (size <= 8){return SVTI_8;}
        if (size <= 16){return SVTI_16;}
        if (size <= 32){return SVTI_32;}
        if (size <= 64){return SVTI_64;}
        return SVTI_64M;

    }

    std::string SVT_toUnitType(SIM_VALREP_TYPE_ALL svt){

        if (svt.type == SVT_U64M){
            return "UintX<" + std::to_string(svt.sub_type) + ">";
        }

        int idx = static_cast<int>(svt.type);
        assert(idx < static_cast<int>(SVT_CNT));
        std::string mapper[SVT_CNT] = {"uint8_t", "uint16_t",
                                       "uint32_t", "uint64_t"};
        return mapper[idx];
    }

    std::string SVTI_toUnitType(SIM_VALREP_TYPE_I_ALL svt){
        if (static_cast<int>(svt.type) == static_cast<int>(SVT_U64M)){
            mf_assert(false, "for now > 64 bit value can't support sign input");
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




    int get_svt_max_bit_size(SIM_VALREP_TYPE_ALL svt){
        if (svt.type == SVT_U64M){
            return bit_size_of_ull * svt.sub_type;
        }
        int idx = svt.type;
        return 8* (1 << idx);
    }

    int get_arr_size(int size){
        return (size + bit_size_of_ull - 1) / bit_size_of_ull;
    }

    SIM_VALREP_TYPE_I_ALL cvt_val_rep_type(SIM_VALREP_TYPE_ALL svt){
        int sub_type_clone = svt.sub_type;
        SIM_VALREP_TYPE_INT converted_type = static_cast<SIM_VALREP_TYPE_INT>(svt.type);

        SIM_VALREP_TYPE_I_ALL ret(converted_type, sub_type_clone);
        return ret;
    }





}
