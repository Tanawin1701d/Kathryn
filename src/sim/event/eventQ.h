//
// Created by tanawin on 14/1/2567.
//

#ifndef KATHRYN_EVENTQ_H
#define KATHRYN_EVENTQ_H

#include "cassert"
#include "queue"
#include "cstdio"
#include "sim/event/eventBase.h"

namespace kathryn{

    class eventQueueCmp{
    public:
        bool operator()(EventBase* lhs, EventBase* rhs){
            return (*lhs) < (*rhs);
        }
    };

    class EventQ {

    private:

        CYCLE lastPopCycle = -1;
        std::priorityQueue<EventBase*, std::vector<EventBase*>, eventQueueCmp> eventQueue;

    public:
        /** add event to the queue*/
        void addEvent(EventBase* event);
        void removeEvent(EventBase* event);
        /**caution! Event Base must be delete if event based is need*/
        EventBase*getNextEventPtr();
        /**pop event*/
        void popEvent();
        /** check empty*/
        bool isEmpty();
        /** check size*/
        sizeT getSize();
        /** get vector @ same cycle */
        std::vector<EventBase*> getAndPopNextSameCycleEvent();
        /** get last popCycle*/
        CYCLE getLastPopCycle() const;
        /** clear eventQ*/
        void reset();




    };

}

#endif //KATHRYN_EVENTQ_H
