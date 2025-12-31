//
// Created by tanawin on 6/10/2024.
//

#ifndef src_sim_logicRep_VALREPGEN_H
#define src_sim_logicRep_VALREPGEN_H
#include "valRep.h"

namespace kathryn{


    ValRepBase bin(int sz, const std::string& value);
    ValRepBase hex(int sz, const std::string& value);

    ValRepBase genWithBase(int baseRange, int sz, const std::string& value);


}

#endif //src/sim/logicRep_VALREPGEN_H
