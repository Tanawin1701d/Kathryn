//
// Created by tanawin on 26/11/25.
//

#include "updateEvent.h"

#include "model/controller/asmMode.h"
#include "model/flowBlock/abstract/logicHelper.h"


namespace kathryn{


    UpdateEventBasic* createUEHelper(Operable*  value,
                                     Slice      sl,
                                     int        priority,
                                     CLOCK_MODE cm,
                                     bool       autoPriority){

        assert(value != nullptr);
        auto* event = new UpdateEventBasic(value, sl, priority, cm);
        if (autoPriority){
            int retrievedPriority = GET_ASM_PRI_VAL();
            event->setPriority(retrievedPriority);
        }
        return event;
    }

    UpdateEventCond* createUEHelper(Operable* cond,
                                    Operable*        state,
                                    UpdateEventBase* ueb){
        auto* conEvent = new UpdateEventCond();
        Operable* condition = addLogicWithOutput(cond, state, BITWISE_AND);
        conEvent->addSubStmt(condition, ueb);
        return conEvent;
    }

    UpdateEventBase* createUEHelper(Operable* cond,
                                    Operable* state,
                                    Operable* value,
                                    Slice sl,
                                    int priority,
                                    CLOCK_MODE cm,
                                    bool autoPriority){

        /*
         *
         * create  base priority
         *
         */
        UpdateEventBasic* event = createUEHelper(value, sl, priority, cm, autoPriority);

        if ((cond == nullptr) && (state == nullptr)){
            return event;
        }

        ////// build condition event if it is needed
        auto* conEvent = createUEHelper(cond, state, event);
        /// the zif will inherit the priority by itself

        return conEvent;

    }

    UpdateEventCond* createMuxUEHelper(UpdateEventBase* left,
                                       UpdateEventBase* right,
                                       Operable* selectLeft){

        assert(selectLeft != nullptr);
        assert(selectLeft->getOperableSlice().getSize() == 1);
        auto uec = new UpdateEventCond();
        uec->addSubStmt(selectLeft, left);
        uec->addSubStmt(nullptr, right);
        return uec;

    }

    bool compareUE(const UpdateEventBase* lhs, const UpdateEventBase* rhs){
        assert(lhs != nullptr);
        assert(rhs != nullptr);
        return (*lhs) < (*rhs);
    }

    UpdatePool UpdatePool::clone(){

        std::vector<UpdateEventBase*> newEvents;
        for (UpdateEventBase* event: events){
            newEvents.push_back(event->clone());
        }
        UpdatePool ret;
        ret.events = newEvents;
        return ret;

    }

    void UpdatePool::clean(){
        for (UpdateEventBase* event: events){
            delete event;
        }
    }

    Operable* UpdatePool::checkShortCircuitProxy() const{
        Operable* result = nullptr;
        for (UpdateEventBase* ueb: events){
            result = ueb->checkShortCircuitProxy();
            if (result != nullptr){
                return result;
            }
        }
        return result;
    }

}
