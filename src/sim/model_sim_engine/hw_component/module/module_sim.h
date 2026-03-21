//
// Created by tanawin on 31/5/2024.
//

#ifndef MODEL_SIMITF_HWCOMPONENT_MODULESIMENGINE_H
#define MODEL_SIMITF_HWCOMPONENT_MODULESIMENGINE_H
#include "sim/model_sim_engine/base/model_proxy.h"
#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"
#include "sim/model_sim_engine/flow_block/flow_base_sim.h"

#include "util/file_writer/file_writer_base.h"


namespace kathryn{

    class Module;
    class LogicSimEngine;
    class ModuleSimEngine{
    protected:
        Module*         _module = nullptr;
    public:
        explicit ModuleSimEngine(Module* module); //// todo module
        /** create zone for hardware element*/
        void                          proxy_build_init              ();
        std::vector<ModelProxyBuild*> recruit_for_create_var         ();
        std::vector<ModelProxyBuild*> recruit_for_regis_var          (); ////// register
        std::vector<ModelProxyBuild*> recruit_for_main_op_volatile    (); ///// must do topologysort
        std::vector<ModelProxyBuild*> recruit_for_main_op_non_volatile ();
        std::vector<ModelProxyBuild*> recruit_for_finalize_op        ();
        std::vector<LogicSimEngine*>  recruit_for_vcd_var            (); ////// register
        //std::vector<LogicSimEngine*>  recruit_all_logic_sim_engine    (); ///// for vcd_write
        /** create zone for perf element*/
        std::vector<FlowBaseSimEngine*> recruit_perf             ();

        ////// state_full u state_less u mem_blk u mem_elh(R) u mem_elh(W)
        void recruit_state_full_ele(std::vector<ModelProxyBuild*>& result);
        void recruit_state_less_ele(std::vector<ModelProxyBuild*>& result);
        void recruit_mem_blk      (std::vector<ModelProxyBuild*>& result);
        void recruit_mem_elh      (std::vector<ModelProxyBuild*>& result, bool is_read_mode);

        void recruit_from_sp_reg   (std::vector<ModelProxyBuild*>& result);



        /** retrieve zone*/
        void retrieve_init(ProxySimEventBase* sim_event_base);

        /** template zone*/
        template<typename T>
        void recruit_from_sub_module(std::vector<T*>& result, std::vector<T*> (ModuleSimEngine::*func)());

        template<typename S, typename T>
        void recruit_from_vector(
            std::vector<S*>& result,
            std::vector<T>& ele_vec);

        template<typename T>
        void retrieve_init_from_vector(ProxySimEventBase* sim_event_base,std::vector<T*>& ele_vec);


    };

    class ModuleSimEngineInterface{
    public:
        virtual ~ModuleSimEngineInterface() = default;
        virtual ModuleSimEngine* get_sim_engine_ptr() = 0;
    };

}

#endif //MODEL_SIMITF_HWCOMPONENT_MODULESIMENGINE_H
