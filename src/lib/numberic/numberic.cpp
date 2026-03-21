//
// Created by tanawin on 16/7/2024.
//

#include "numberic.h"

namespace kathryn{

    Operable& ext(Operable& value, int target_size, Operable& sign){
        assert(target_size > value.get_operable_slice().get_size());
        assert(value.get_operable_slice().get_size() > 0);
        assert(sign.get_operable_slice().get_size() == 1);

        int amt_to_extend = target_size - value.get_operable_slice().get_size();
                                /////   v---- lsb                  v----- msb
        return g_man_internal_read_only( {&value, &((sign & 1).ext_b(amt_to_extend))});

    }

}