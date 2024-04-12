//
// Created by tanawin on 11/4/2567.
//

#include "updateEvent.h"

namespace kathryn{

    ull GLOBAL_UPDATE_EVENT_ID = 0;


    UpdateEvent::UpdateEvent(Operable* cond,
                             Operable* state,
                             Operable* val,
                             Slice desSlice,
                             int pri):
    srcUpdateCondition(cond),
    srcUpdateState(state),
    srcUpdateValue(val),
    desUpdateSlice(desSlice),
    priority(pri),
    updateIdx(GLOBAL_UPDATE_EVENT_ID++)
    {}
}