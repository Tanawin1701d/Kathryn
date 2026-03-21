//
// Created by tanawin on 14/1/2567.
//

#ifndef KATHRYN_EVENTQ_H
#define KATHRYN_EVENTQ_H

#include "cassert"
#include "queue"
#include "cstdio"
#include "sim/event/event_base.h"

namespace kathryn{

    class event_queue_cmp{
    public:
        bool operator()(EventBase* lhs, EventBase* rhs){
            return (*lhs) < (*rhs);
        }
    };

    class EventQ {

    private:

        CYCLE last_pop_cycle = -1;
        std::priority_queue<EventBase*, std::vector<EventBase*>, event_queue_cmp> event_queue;

    public:
        /** add event to the queue*/
        void add_event(EventBase* event);
        void remove_event(EventBase* event);
        /**caution! Event Base must be delete if event based is need*/
        EventBase*get_next_event_ptr();
        /**pop event*/
        void pop_event();
        /** check empty*/
        bool is_empty();
        /** check size*/
        size_t get_size();
        /** get vector @ same cycle */
        std::vector<EventBase*> get_and_pop_next_same_cycle_event();
        /** get last pop_cycle*/
        CYCLE get_last_pop_cycle() const;
        /** clear event_q*/
        void reset();




    };

}

#endif //KATHRYN_EVENTQ_H
