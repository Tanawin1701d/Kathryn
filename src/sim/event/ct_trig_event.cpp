//
// Created by tanawin on 29/2/2567.
//

#include "ct_trig_event.h"
#include "sim/interface/sim_interface.h"



namespace kathryn{

    ConcreteTriggerEvent::ConcreteTriggerEvent(CYCLE target_cycle,
                                               SimInterface* sim_interface_master,
                                               std::function<bool()> condition_trigger,
                                               int prority):
    EventBase(target_cycle,
              prority,
              false),
              _simInterfaceMaster(sim_interface_master),
              _conditionTrigger(condition_trigger){
        get_sim_controller_ptr()->add_event(this);
    }

    void ConcreteTriggerEvent::sim_start_cur_cycle_neg() {
        start_sim_cur_event.notify(get_cur_cycle()); //// notify the run aside thread
        finish_sim_cur_event.wait(get_cur_cycle());  //// wait for the run aside thread to finish
    }

    void ConcreteTriggerEvent::sim_start_next_cycle_pos(){
        start_end_cycle_event.notify(get_cur_cycle()); //// notify the run aside thread
        finish_end_cycle_event.wait(get_cur_cycle());  //// wait for the run aside thread to finish
    }



    EventBase* ConcreteTriggerEvent::gen_next_event(){
        if (stop){return nullptr;}
        _targetCycle = next_cycle;
        return this;
    }




}