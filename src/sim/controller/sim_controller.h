//
// Created by tanawin on 15/1/2567.
//

#ifndef KATHRYN_SIMCONTROLLER_H
#define KATHRYN_SIMCONTROLLER_H

#include "iostream"
#include "mutex"

#include "sim/event/event_base.h"
#include "sim/event/event_q.h"
#include "abstract/main_controlable.h"
#include "sim/model_sim_engine/base/trace_event.h"
#include "sim/logic_rep/val_rep.h"
#include "sim/logic_rep/val_rep_gen.h"

namespace kathryn{

    class ProxySimEventBase;
    class Wire;
    class SimController: public MainControlable{
    private:
        std::mutex              _rsMtx;
        CYCLE                   _limitCycle    =  1;
        CYCLE                   _curCycle      = -1;
        EventQ                  event_q;
        bool                    stop_mark        = false;
        ///// the trace idx is the idx of vector
        std::vector<TraceEvent>* _mdTraceMap   = nullptr;
        CYCLE*                   _amtLrLimUser = nullptr; //// amount cylce that user limit

        void collect_data();

    public:

        explicit SimController();
        void  start() override;
        void  reset() override;
        void  clean() override;
        void  add_event(EventBase* event);
        void  save_data();
        void  set_limit_cycle(CYCLE lmt_cycle);
        void  set_trigger_map(std::vector<TraceEvent>* md_trace_map);
        void  set_lr_lim_user(CYCLE* amt_lr_lim_user);
        void  stop_sim();
        CYCLE get_cur_cycle();
        void  lock();
        void  unlock();


    };

    SimController*get_sim_controller_ptr();

}

#endif //KATHRYN_SIMCONTROLLER_H
