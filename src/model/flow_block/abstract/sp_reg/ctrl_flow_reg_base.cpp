//
// Created by tanawin on 14/1/2567.
//


#include "ctrl_flow_reg_base.h"

#include "model/flow_block/abstract/logic_helper.h"

namespace kathryn{

    std::string sp_reg_type_to_str(SP_REG_TYPE sp_reg_type){
        std::string mapper[SP_CNT_REG] = {
            "SP_STATE_REG",
            "SP_SYNC_REG",
            "SP_COND_WAIT_REG",
            "SP_CYCLE_WAIT_REG"
        };
        assert(sp_reg_type < SP_CNT_REG);
        return mapper[sp_reg_type];
    }


    UpdateEventBase* CtrlFlowRegBase::create_ue(Operable* cond,
                                               Operable* state,
                                               Operable* value,
                                               Slice sl,
                                               int priority,
                                               CLOCK_MODE cm){

        UpdateEventBase* con_event = create_ue_helper(cond, state, value, sl, priority, cm, false);
        return con_event;

    }


}
