//
// Created by tanawin on 14/1/2567.
//


#include "eventQ.h"

namespace kathryn{


    void EventQ::addEvent(EventBase *event) {
        assert(event != nullptr);
        assert(event->getCurCycle() >= lastPopCycle);
        eventQueue.push(event);
    }

    void EventQ::removeEvent(EventBase *event) {
        /* TODO remove event is not implement in this version
         * because it requires custom queue to handle this operation
         * */
        assert(false);
    }

    EventBase *EventQ::getNextEvent() {
        assert(!eventQueue.empty());
        return eventQueue.top();
    }

    void EventQ::popEvent() {
        assert(!eventQueue.empty());
        auto topEvent = eventQueue.top();
        lastPopCycle = topEvent->getCurCycle();
        eventQueue.pop();
    }

    bool EventQ::isEmpty() {
        return eventQueue.empty();
    }

    size_t EventQ::getSize() {
        return eventQueue.size();
    }

    std::vector<EventBase*> EventQ::getAndPopNextSameCycleEvent() {

        assert(!eventQueue.empty());
        std::vector<EventBase*> preRetRes;
        CYCLE selectedCycle = getNextEvent()->getCurCycle();

        while ( (!eventQueue.empty()) &&
                (getNextEvent()->getCurCycle() == selectedCycle) ){
            preRetRes.push_back(getNextEvent());
            popEvent();
        }
        return preRetRes;
    }

    CYCLE EventQ::getLastPopCycle() const {
        return lastPopCycle;
    }

    void EventQ::reset() {
        lastPopCycle = -1;
        eventQueue =std::priority_queue<
                            EventBase*,
                            std::vector<EventBase*>,
                            eventQueueCmp
                            >();
    }



}