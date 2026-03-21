//
// Created by tanawin on 25/1/2567.
//

#ifndef KATHRYN_USEREVENT_H
#define KATHRYN_USEREVENT_H

#include "utility"

#include "event_base.h"
#include "sim/logic_rep/val_rep.h"


/*** sim_agent*/

/** if you wish to use this macro*/
#define sim sim_agent << [&](UserEvent& sim_agent)
#define inc_cycle(cycle) sim_agent.iterate_orchest_cycle(cycle)
#define back_cycle()     sim_agent.jump_to_end_cycle()
#define set_cycle(cycle) sim_agent.set_orchest_cycle(cycle)

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
     /*** sub user_event*/
     std::vector<UserEvent*> _subEvents;

 public:
     UserEvent(std::function<void(UserEvent&)> active_func,
               UserEvent* parent,
               int pri);

     UserEvent(std::function<void(UserEvent&)> active_func,
               CYCLE sim_cycle,
               int pri);

     UserEvent();

     void sim_start_long_run_cycle() override{assert(false);}
     /**
       * compute value that will be assigned in this cycle
       **/
     void sim_start_cur_cycle_neg() override{_activeFunc(*this);}
     void sim_start_cur_cycle_pos() override{}

     /**
      * collect data from every compute unit
      * ex for reg wire unit will write data to cmd unit
      **/
     void cur_cycle_collect_data_neg()override{}
     void cur_cycle_collect_data_pos()override{}
     /**
     * compute value for next cycle
     * */
     void sim_start_next_cycle_neg()override{}
     void sim_start_next_cycle_pos()override{}

     void sim_exit_cur_cycle() override{}

     bool need_to_delete() override{return true;}

     void add_sub_event(UserEvent* event) {
         assert(event != nullptr);
         _subEvents.push_back(event);
     }


     CYCLE get_orchest_cycle() const {
         return _orchestCycle;
     }

     void iterate_orchest_cycle(CYCLE cycle_amt){
         _orchestPriority = SIM_USER_PRIO_FRONT_CYCLE;
         _orchestCycle += cycle_amt;

     }

     void set_orchest_cycle(CYCLE cycle_amt){
         _orchestPriority = SIM_USER_PRIO_FRONT_CYCLE;
         _orchestCycle = cycle_amt;
     }

     void jump_to_end_cycle(){
         _orchestPriority = SIM_USER_PRIO_BACK_CYCLE;
     }

     void operator << (std::function<void(UserEvent&)> sim_behaviour);


 };






}

#endif //KATHRYN_USEREVENT_H
