//
// Created by tanawin on 6/10/2024.
//

#ifndef src_sim_logicRep_VALREPGEN_H
#define src_sim_logicRep_VALREPGEN_H
#include "val_rep.h"

namespace kathryn{


    ValRepBase bin(int sz, const std::string& value);
    ValRepBase hex(int sz, const std::string& value);

    ValRepBase gen_with_base(int base_range, int sz, const std::string& value);


}

#endif //src/sim/logicRep_VALREPGEN_H
