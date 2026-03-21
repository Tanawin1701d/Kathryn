//
// Created by tanawin on 19/7/2024.
//

#include "gen_helper.h"
#include "logic_sim_engine.h"


namespace kathryn{

    ValR get_src_opr_from_opr(Operable* opr1){
        assert(opr1 != nullptr);
        return opr1->get_exact_operable().
                get_logic_sim_engine_from_opr_ptr()->gen_src_opr();
    }


    ValR get_sliced_src_opr_from_opr(Operable* opr1){
        assert(opr1 != nullptr);
        LogicSimEngine* sim_engine = opr1->get_logic_sim_engine_from_opr_ptr();
        ////// slice to operabe slice
        return get_sliced_src_opr_from_opr(opr1, sim_engine->getValR_Type());
    }

    ValR get_sliced_src_opr_from_opr(Operable* opr1, SIM_VALREP_TYPE_ALL des_field){
        assert(opr1 != nullptr);
        LogicSimEngine* sim_engine = opr1->get_logic_sim_engine_from_opr_ptr();
        Slice needed_slice = opr1->get_operable_slice();
        ////// slice to operabe slice
        return sim_engine->gen_sliced_opr_to(needed_slice, des_field);
    }

    ValR get_sliced_and_shift_src_opr_from_opr(Operable* opr1, Slice des_slice, SIM_VALREP_TYPE_ALL des_field){
        assert(opr1 != nullptr);
        LogicSimEngine* sim_engine = opr1->get_logic_sim_engine_from_opr_ptr();
        Slice needed_slice = opr1->get_operable_slice();
        ////// slice to operabe slice
        return sim_engine->gen_sliced_opr_and_shift(des_slice, needed_slice, des_field);
    }

    ull create_mask(Slice mask_slice){
        assert((mask_slice.check_valid_slice()) && (mask_slice.stop <= bit_size_of_ull));
        assert(mask_slice.start < bit_size_of_ull);
        int size = mask_slice.get_size();
        return (size == bit_size_of_ull) ? -1 : ((((ull)1) << size) - 1);
    }


}