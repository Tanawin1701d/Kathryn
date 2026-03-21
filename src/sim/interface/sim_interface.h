//
// Created by tanawin on 25/1/2567.
//

#ifndef KATHRYN_SIMINTERFACE_H
#define KATHRYN_SIMINTERFACE_H

#include "condition_variable"
#include "thread"
#include "sim/model_sim_engine/base/proxy_build_mng.h"
#include "model/controller/controller.h"
#include "sim/event/user_event.h"
#include "sim/controller/sim_controller.h"
#include "sim/event/ct_trig_event.h"
#include "sim/model_sim_engine/base/trace_event.h"
#include "util/term_color/term_color.h"

/////#define sim agent << [&]()

namespace kathryn{




    class SimInterface{
        friend class ConcreteTriggerEvent;
    protected:
        SimProxyBuildMode       _simProxyBuildMode;
        VcdWriter*              _vcdWriter      = nullptr;
        FlowWriter*             _flowWriter     = nullptr;
        ProxySimEventBase*      _modelSimEvent  = nullptr;
        std::vector<UserEvent*> UserSimEvents;
        CYCLE                   _limitCycle = 0;
        UserEvent               sim_agent;/** sim agent base can't change name*/
        ProxyBuildMng           _proxyBuildMng;

        //////////////////////////
        /**long range simulation*/
        //////////////////////////
        std::vector<TraceEvent> _traceEvents;
        bool                    _requireLRC       = false;
        CYCLE                   _nextLimitAmtLRC  = INT64_MAX; //// next limit amount long run cyclc
        ///////////////////
        /**concrete sim*/
        ///////////////////
        std::unique_ptr<std::thread> con_thread;
        bool                         require_con_sim  = true;
        ConcreteTriggerEvent*        last_ct_trigger = nullptr;
        CYCLE                        con_cur_cycle_used = 2;

        void sim_start_con_sim();
        /**for now we will not support Condition*/
        void con_cycle_base(CYCLE start_cycle);
        void con_cycle(CYCLE start_cycle);
        void con_next_cycle(CYCLE amt_cycle);
        void con_end_cycle();
        //// to initialize system and finalize system
        void describe_con_wrapper();
        //////////////////////////////////////////////////////
        ///////////////////
        ////**trigger mng*/
        enum SIM_INTERFACE_EVENT{
            EXIT_SIM
        };
        void set_next_limit_amt_lrc(CYCLE amt_cycle);
        void trig(Operable& opr1, std::function<void()> callback);
        void trig(Operable& opr1, SIM_INTERFACE_EVENT event);
        void describe_model_trigger_wrapper();


    public:
        explicit SimInterface(CYCLE limit_cycle,
                              std::string vcd_file_path,
                              std::string profile_file_path,
                              std::string gen_file_name = "proxy_sim_auto_gen.cpp",
                              SimProxyBuildMode sim_proxy_build_mode =
                                    SimProxyBuildMode::SPB_GEN |
                                    SimProxyBuildMode::SPB_COMPILE |
                                    SimProxyBuildMode::SPB_RUN,
                              bool require_lrc = false, ///// this will enable model sim event start with long range simulator
                              bool req_inline  = true,
                              int  op_level = 3
                              );

        virtual ~SimInterface();

        void sim_start();
        /**describe default value*/
        virtual void    describe_def();
        /**describe model trigger value*/
        virtual void    describe_model_trigger(){};
        /**describe discreate event*/
        virtual void describe(){};
        /**describe concrete event*/
        virtual void describe_con(){};

        /** test value helper*/
        void test_and_print(const std::string& test_name, ValRepBase& sim_val, ValRepBase& rhs);
        void test_and_print(const std::string& test_name, ull sim_val, ull expect);
        /** build proxy SimEvent*/
        void create_model_sim_event();
        void init_perf_col();
        void final_perf_col();
        /** require con sim*/
        void disable_con_sim(){require_con_sim = false;} ///// it will not lunch con thead simulator
        /** require LRC (this can be invoked before simulation is start only)*/
        void enable_lrc(){_requireLRC = true;}

        ProxySimEventBase*get_proxy_sim_event_ptr(){return _modelSimEvent;}




    };

}

#endif //KATHRYN_SIMINTERFACE_H
