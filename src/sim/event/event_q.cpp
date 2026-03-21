//
// Created by tanawin on 14/1/2567.
//


#include "event_q.h"

namespace kathryn{


    void EventQ::add_event(EventBase *event) {
        assert(event != nullptr);
        assert(event->get_cur_cycle() >= last_pop_cycle);
        event_queue.push(event);
    }

    void EventQ::remove_event(EventBase *event) {
        /* TODO remove event is not implement in this version
         * because it requires custom queue to handle this operation
         * */
        assert(false);
    }

    EventBase *EventQ::get_next_event_ptr() {
        assert(!event_queue.empty());
        return event_queue.top();
    }

    void EventQ::pop_event() {
        assert(!event_queue.empty());
        auto top_event = event_queue.top();
        last_pop_cycle = top_event->get_cur_cycle();
        event_queue.pop();
    }

    bool EventQ::is_empty() {
        return event_queue.empty();
    }

    size_t EventQ::get_size() {
        return event_queue.size();
    }

    std::vector<EventBase*> EventQ::get_and_pop_next_same_cycle_event() {

        assert(!event_queue.empty());
        std::vector<EventBase*> pre_ret_res;
        CYCLE selected_cycle = get_next_event_ptr()->get_cur_cycle();

        while ( (!event_queue.empty()) &&
                (get_next_event_ptr()->get_cur_cycle() == selected_cycle) ){
            pre_ret_res.push_back(get_next_event_ptr());
            pop_event();
        }
        return pre_ret_res;
    }

    CYCLE EventQ::get_last_pop_cycle() const {
        return last_pop_cycle;
    }

    void EventQ::reset() {
        last_pop_cycle = -1;
        event_queue =std::priority_queue<
                            EventBase*,
                            std::vector<EventBase*>,
                            event_queue_cmp
                            >();
    }



}