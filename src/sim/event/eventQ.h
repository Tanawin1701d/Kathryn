//
// Created by tanawin on 14/1/2567.
//

#ifndef KATHRYN_EVENTQ_H
#define KATHRYN_EVENTQ_H

#include <cassert>
#include <queue>
#include <cstdio>
#include "sim/event/eventBase.h"

namespace kathryn{

    class EventQ {

    private:
        CYCLE lastPopCycle = 0;
        std::priority_queue<EventBase *> eventQueue;

    public:
        /** add event to the queue*/
        void addEvent(EventBase* event);
        void removeEvent(EventBase* event);
        /**caution! Event Base must be delete if event based is need*/
        EventBase* getNextEvent();
        /**pop event*/
        void popEvent();
        /** check empty*/
        bool isEmpty();
        /** check size*/
        size_t getSize();
        /** get vector @ same cycle */
        std::vector<EventBase*> getAndPopNextSameCycleEvent();
        /** get last popCycle*/
        CYCLE getLastPopCycle() const;

    };

}

#endif //KATHRYN_EVENTQ_H
