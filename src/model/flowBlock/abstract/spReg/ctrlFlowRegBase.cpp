//
// Created by tanawin on 14/1/2567.
//


#include "ctrlFlowRegBase.h"

#include "model/flowBlock/abstract/logicHelper.h"

namespace kathryn{

    std::string spRegTypeToStr(SP_REG_TYPE spRegType){
        std::string mapper[SP_CNT_REG] = {
            "SP_STATE_REG",
            "SP_SYNC_REG",
            "SP_COND_WAIT_REG",
            "SP_CYCLE_WAIT_REG"
        };
        assert(spRegType < SP_CNT_REG);
        return mapper[spRegType];
    }


    UpdateEventBase* CtrlFlowRegBase::createUe(Operable* cond,
                                               Operable* state,
                                               Operable* value,
                                               Slice sl,
                                               int priority,
                                               CLOCK_MODE cm){

        UpdateEventBase* conEvent = createUeHelper(cond, state, value, sl, priority, cm, false);
        return conEvent;

    }


}
