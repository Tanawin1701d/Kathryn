//
// Created by tanawin on 25/1/2567.
//

#ifndef KATHRYN_USEREVENT_H
#define KATHRYN_USEREVENT_H

#include <utility>

#include "eventBase.h"
#include "sim/logicRep/valRep.h"


/*** simAgent*/

/** if you wish to use this macro*/
#define sim simAgent << [&](UserEvent& simAgent)
#define incCycle(cycle) simAgent.iterateOrchestCycle(cycle)
#define backCycle()     simAgent.jumpToEndCycle()
#define setCycle(cycle) simAgent.setOrchestCycle(cycle)

namespace kathryn{

/**
 * procedure that user need to run in each cycle
 * **/

 class UserEvent: public EventBase{
 private:
     CYCLE      _orchestCycle = -1;
     int        _orchestPriority     = SIM_USER_PRIO_FRONT_CYCLE;
     UserEvent* _parent = nullptr;
     /*** core function to be used*/
     std::function<void(UserEvent&)> _activeFunc;
     /*** sub userEvent*/
     std::vector<UserEvent*> _subEvents;

 public:
     UserEvent(std::function<void(UserEvent&)> activeFunc,
               UserEvent* parent,
               int pri);

     UserEvent(std::function<void(UserEvent&)> activeFunc,
               CYCLE simCycle,
               int pri);

     UserEvent();

     void simStartLongRunCycle() override{assert(false);}

     void simStartCurCycle() override{
         _activeFunc(*this);
     }

     void curCycleCollectData() override{}

     void simStartNextCycle() override{}

     void simExitCurCycle() override{}

     bool needToDelete() override{return true;}

     void addSubEvent(UserEvent* event) {
         assert(event != nullptr);
         _subEvents.push_back(event);
     }


     CYCLE getOrchestCycle() const {
         return _orchestCycle;
     }

     void iterateOrchestCycle(CYCLE cycleAmt){
         _orchestPriority = SIM_USER_PRIO_FRONT_CYCLE;
         _orchestCycle += cycleAmt;

     }

     void setOrchestCycle(CYCLE cycleAmt){
         _orchestPriority = SIM_USER_PRIO_FRONT_CYCLE;
         _orchestCycle = cycleAmt;
     }

     void jumpToEndCycle(){
         _orchestPriority = SIM_USER_PRIO_BACK_CYCLE;
     }

     void operator << (std::function<void(UserEvent&)> simBehaviour);


 };






}

#endif //KATHRYN_USEREVENT_H
