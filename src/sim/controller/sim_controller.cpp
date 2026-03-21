//
// Created by tanawin on 15/1/2567.
//

#include "sim_controller.h"

namespace kathryn{

    SimController::SimController():
    _limitCycle(0){}

    void SimController::collect_data() {}

    void SimController::start() {


        lock();
        while ( (!event_q.is_empty()) &&
                (event_q.get_next_event_ptr()->get_cur_cycle() <= _limitCycle)
        ){

            EventBase* next_event = event_q.get_next_event_ptr();
            /**we are sure that next_event is valid due to while loop at the top*/
            assert(_curCycle != next_event->get_cur_cycle()); //// check therer is no same cycle used
            ////std::cout << "cur_cycle is " << _curCycle << std::endl;
            _curCycle = next_event->get_cur_cycle();
            std::vector<EventBase*> _curCycleEvents;

            ///// std::cout << "cur cycle" << _curCycle << std::endl;

            /**
             *
             * start cur cycle which allows same cycle queue adding
             *
             * */

            while ((!event_q.is_empty()) && (event_q.get_next_event_ptr()->get_cur_cycle() == _curCycle)){
                _curCycleEvents.push_back(event_q.get_next_event_ptr());
                event_q.pop_event();
            }

            CYCLE amt_user_limit     = (_amtLrLimUser == nullptr)? INT64_MAX: *_amtLrLimUser;
            CYCLE next_event_occur_at = event_q.is_empty()          ? INT64_MAX: event_q.get_next_event_ptr()->get_cur_cycle();
            /**
             * all event is simulated. For now, This cycle is stable.
             * */
            unlock();
            //////// neg edge first
            for (auto* cur_event: _curCycleEvents){
                cur_event->sim_start_cur_cycle_neg();
            }
            for (auto* cur_event: _curCycleEvents){
                cur_event->cur_cycle_collect_data_neg();
            }
            for (auto* cur_event: _curCycleEvents){
                cur_event->sim_start_next_cycle_neg();
            }
            //////// pos edge first
            for (auto* cur_event: _curCycleEvents){
                cur_event->sim_start_cur_cycle_pos();
            }
            for (auto* cur_event: _curCycleEvents){
                cur_event->cur_cycle_collect_data_pos();
            }
            for (auto* cur_event: _curCycleEvents){
                cur_event->sim_start_next_cycle_pos();
            }
            //////// long run cycle
            for (auto* cur_event: _curCycleEvents){
                int lrc = 0; //// long range counter
                if (cur_event->is_long_rage_sim()){
                    ///////////// intepret the cycle to run
                    lrc++;
                    CYCLE amt_limit_by_queue  = next_event_occur_at - cur_event->get_cur_cycle();
                    assert(amt_limit_by_queue > 0 && amt_user_limit > 0);
                    cur_event->set_long_range_sim(std::min(amt_user_limit, amt_limit_by_queue));
                    ///////////// start running
                    cur_event->sim_start_long_run_cycle();
                    for (int call_back_idx = 0; call_back_idx < cur_event->get_call_back_amt(); call_back_idx++){
                        int call_back_no = cur_event->get_call_back_no(call_back_idx);
                        assert(call_back_no < _mdTraceMap->size());
                        (*_mdTraceMap)[call_back_no].exec_call_back();
                    }
                }
                assert(lrc <= 1); ///// we must have only or non for long rage sim
            }
            //////// populate next cycle
            for (auto cur_event: _curCycleEvents){
                EventBase* after_event = cur_event->gen_next_event();
                if (after_event != nullptr){
                    add_event(after_event);
                }
                if (cur_event->need_to_delete()){
                    delete cur_event;
                }
            }
            lock();

            if (stop_mark){
                break;
            }

        }
        unlock();

    }

    void SimController::add_event(EventBase *event) {
        assert(event != nullptr);
        lock();
        event_q.add_event(event);
        unlock();
    }

    void SimController::save_data() {
        assert(false);
    }

    void SimController::set_limit_cycle(CYCLE lmt_cycle){
        lock();
        _limitCycle = lmt_cycle;
        unlock();
    }

    void SimController::reset(){
        lock();
        _limitCycle = 1;
        event_q.reset();
        stop_mark      = false;
        _mdTraceMap   = nullptr;
        _amtLrLimUser = nullptr;
        unlock();
    }

    void SimController::clean(){
        reset();
    }

    CYCLE SimController::get_cur_cycle(){
        lock();
        CYCLE cpy_cycle = _curCycle;
        unlock();
        return cpy_cycle;
    }

    void SimController::set_trigger_map(std::vector<TraceEvent>* md_trace_map){
        lock();
        assert(md_trace_map != nullptr);
        _mdTraceMap = md_trace_map;
        unlock();
    }

    void SimController::set_lr_lim_user(CYCLE* amt_lr_lim_user){
        assert(amt_lr_lim_user != nullptr);
        lock();
        _amtLrLimUser = amt_lr_lim_user;
        unlock();
    }

    void SimController::stop_sim(){
        lock();
        stop_mark = true;
        unlock();
    }

    void SimController::lock(){
        _rsMtx.lock();
    }

    void SimController::unlock(){
        _rsMtx.unlock();
    }






    /***
     *
     *
     * get sim controller (lazy initialization)
     *
     * **/

    SimController* sim_ctrl = nullptr;

    SimController*get_sim_controller_ptr(){

        if (sim_ctrl == nullptr){
            sim_ctrl = new SimController();
        }
        return sim_ctrl;

    }

}