//
// Created by tanawin on 19/7/2024.
//

#ifndef SRCOPRHELPER_H
#define SRCOPRHELPER_H

#include "string"

#include "model/hw_component/abstract/assignable.h"
#include "model/hw_component/abstract/slice.h"
#include "sim/model_sim_engine/base/sim_val_type.h"


namespace kathryn{
    ValR get_src_opr_from_opr      (Operable* opr1);
    ValR get_sliced_src_opr_from_opr(Operable* opr1);
    ValR get_sliced_src_opr_from_opr(Operable* opr1, SIM_VALREP_TYPE_ALL des_field ); ///// the des field that the sliced should be placed
    ValR get_sliced_and_shift_src_opr_from_opr
                               (Operable* opr1, Slice des_slice,
                                SIM_VALREP_TYPE_ALL des_field);

    ull create_mask(Slice mask_slice);
}

#endif //SRCOPRHELPER_H
