//
// Created by tanawin on 26/11/25.
//

#include "update_event.h"

#include "model/controller/asm_mode.h"
#include "model/flow_block/abstract/logic_helper.h"


namespace kathryn{


    UpdateEventBasic* create_ue_helper(Operable*  value,
                                     Slice      sl,
                                     int        priority,
                                     CLOCK_MODE cm,
                                     bool       auto_priority){

        assert(value != nullptr);
        auto* event = new UpdateEventBasic(value, sl, priority, cm);
        if (auto_priority){
            int retrieved_priority = GET_ASM_PRI_VAL();
            event->set_priority(retrieved_priority);
        }
        return event;
    }

    UpdateEventCond* create_ue_helper(Operable* cond,
                                    Operable*        state,
                                    UpdateEventBase* ueb){
        auto* con_event = new UpdateEventCond();
        Operable* condition = add_logic_with_output(cond, state, BITWISE_AND);
        con_event->add_sub_stmt(condition, ueb);
        return con_event;
    }

    UpdateEventBase* create_ue_helper(Operable* cond,
                                    Operable* state,
                                    Operable* value,
                                    Slice sl,
                                    int priority,
                                    CLOCK_MODE cm,
                                    bool auto_priority){

        /*
         *
         * create  base priority
         *
         */
        UpdateEventBasic* event = create_ue_helper(value, sl, priority, cm, auto_priority);

        if ((cond == nullptr) && (state == nullptr)){
            return event;
        }

        ////// build condition event if it is needed
        auto* con_event = create_ue_helper(cond, state, event);
        /// the zif will inherit the priority by itself

        return con_event;

    }

    UpdateEventCond* create_mux_ue_helper(UpdateEventBase* left,
                                       UpdateEventBase* right,
                                       Operable* select_left){

        assert(select_left != nullptr);
        assert(select_left->get_operable_slice().get_size() == 1);
        auto uec = new UpdateEventCond();
        uec->add_sub_stmt(select_left, left);
        uec->add_sub_stmt(nullptr, right);
        return uec;

    }

    bool compare_ue(const UpdateEventBase* lhs, const UpdateEventBase* rhs){
        assert(lhs != nullptr);
        assert(rhs != nullptr);
        return (*lhs) < (*rhs);
    }

    void UpdatePool::sort_events(){
        for (UpdateEventBase* ueb: events){
            assert(ueb != nullptr);
        }
        std::sort(events.begin(), events.end(), [](const UpdateEventBase* lhs, const UpdateEventBase* rhs){
        if (!lhs || !rhs) {
            std::cerr << "NULL IN SORT lhs=" << lhs
                      << " rhs=" << rhs << "\n";
            std::abort();
        }
        return *lhs < *rhs;
    });
    }

    UpdatePool UpdatePool::clone(){

        std::vector<UpdateEventBase*> new_events;
        for (UpdateEventBase* event: events){
            new_events.push_back(event->clone());
        }
        UpdatePool ret;
        ret.events = new_events;
        return ret;

    }

    void UpdatePool::clean(){
        for (UpdateEventBase* event: events){
            delete event;
        }
    }

    Operable* UpdatePool::check_short_circuit_proxy() const{
        Operable* result = nullptr;
        for (UpdateEventBase* ueb: events){
            result = ueb->check_short_circuit_proxy();
            if (result != nullptr){
                return result;
            }
        }
        return result;
    }

    bool UpdatePool::is_clock_mode_consistent() const{

        if (is_empty()){ return true;}
        UpdateEventBase* first = events.front();
        assert(first != nullptr);

        for (UpdateEventBase* base_ptr: events){
            assert(base_ptr != nullptr);
            if (first->get_clk_mode() != base_ptr->get_clk_mode()){
                return false;
            }
        }
        return true;
    }


    CLOCK_MODE UpdatePool::get_clock_mode() const{
        assert(is_clock_mode_consistent());
        if (is_empty()){
            return CLOCK_MODE::CM_CLK_UNUSED;
        }
        return events[0]->get_clk_mode();
    }



}
