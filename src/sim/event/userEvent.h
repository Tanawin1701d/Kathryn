//
// Created by tanawin on 25/1/2567.
//

#ifndef KATHRYN_USEREVENT_H
#define KATHRYN_USEREVENT_H

#include <utility>

#include "eventBase.h"
#include "sim/logicRep/valRep.h"

namespace kathryn{

/**
 * procedure that user need to run in each cycle
 * **/

 class UserEvent: public EventBase{
 private:
     std::function<void(void)> _activeFunc;

 public:
     UserEvent(std::function<void(void)> activeFunc, CYCLE cycle, int pri):
        EventBase(cycle, pri),
        _activeFunc(std::move(activeFunc))
     {
         addNewEvent(this);
     }

     void simStartCurCycle() override{
         _activeFunc();
     }

     void curCycleCollectData() override{}

     void simExitCurCycle() override{}

     bool needToDelete() override{return true;}

 };






}

#endif //KATHRYN_USEREVENT_H
