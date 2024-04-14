//
// Created by tanawin on 14/1/2567.
//


#include"ctrlFlowRegBase.h"

namespace kathryn{

    std::string sp_reg_type_to_str(SP_REG_TYPE spRegType){
        std::string mapper[SP_CNT_REG] = {
            "SP_STATE_REG",
            "SP_SYNC_REG",
            "SP_COND_WAIT_REG",
            "SP_CYCLE_WAIT_REG"
        };
        assert(spRegType < SP_CNT_REG);
        return mapper[spRegType];
    }

}


