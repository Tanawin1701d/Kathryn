//
// Created by tanawin on 29/2/2567.
//

#ifndef KATHRYN_CTTRIGEVENT_H
#define KATHRYN_CTTRIGEVENT_H

#include "condition_variable"
#include "functional"
#include "iostream"
#include "event_base.h"
#include "sim/logic_rep/val_rep.h"

namespace kathryn{

    class SimInterface;
    class ConcreteTriggerEvent: EventBase{

        friend class SimInterface;
        struct SerializeEvent{
            std::mutex  mtx;
            std::condition_variable con_var;
            CYCLE  last_cycle_notified =  -1;


            void notify(CYCLE your_cycle){
                mtx.lock();
                if (your_cycle >= last_cycle_notified){
                    last_cycle_notified = your_cycle;
                }
                mtx.unlock();
                con_var.notify_all();

            }

            void wait(CYCLE your_cycle){

                std::unique_lock<std::mutex> locker(mtx);
                if (your_cycle <= last_cycle_notified){
                    ////// other is ahead of us
                    locker.unlock();
                    return;
                }

                con_var.wait(locker, [&](){ /****std::cout << is_processed << std::endl;****/
                return your_cycle <= last_cycle_notified;});
                mtx.unlock();

            }


        };

    private:

        SimInterface* _simInterfaceMaster = nullptr;
        std::function<bool()>& _conditionTrigger;


        SerializeEvent start_sim_cur_event;
        SerializeEvent finish_sim_cur_event;
        SerializeEvent start_end_cycle_event;
        SerializeEvent finish_end_cycle_event;


        CYCLE next_cycle = -1;
        bool  stop       = false;

    public:
        explicit ConcreteTriggerEvent(CYCLE target_cycle,
                                      SimInterface* sim_interface_master,
                                      std::function<bool()> condition_trigger,
                                      int prority
                                      );

        void sim_start_long_run_cycle() override{assert(false);}

        void sim_start_cur_cycle_neg()    override;
        void sim_start_cur_cycle_pos()    override{}

        void cur_cycle_collect_data_neg() override{}
        void cur_cycle_collect_data_pos() override{}

        void sim_start_next_cycle_neg()   override{}
        void sim_start_next_cycle_pos()   override;


        void sim_exit_cur_cycle() override{};

        void set_future_cycle(CYCLE future_cycle){next_cycle = future_cycle;}

        EventBase* gen_next_event() override;

        void mark_stop(){stop = 1;}

        bool need_to_delete() override{ return stop;}


    };

}

#endif //KATHRYN_CTTRIGEVENT_H
