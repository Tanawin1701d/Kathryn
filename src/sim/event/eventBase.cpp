//
// Created by tanawin on 21/1/2567.
//

#include "eventBase.h"
#include "sim/controller/controller.h"


namespace kathryn{

    void EventBase::addNewEvent(EventBase* newEvent){
        assert(newEvent != nullptr);
        SimController* simCtrl = getSimController();
        simCtrl->addEvent(newEvent);
    }

}