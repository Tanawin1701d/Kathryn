//
// Created by tanawin on 31/5/2024.
//

#include "module_sim.h"

#include "util/type/type_conv.h"
#include "model/hw_component/module/module.h"


namespace kathryn{
    ModuleSimEngine::ModuleSimEngine(Module* module):
        _module(module){
    }

    void ModuleSimEngine::proxy_build_init(){
        std::vector<ModelProxyBuild*> result;

        /////////// recruit all variable
        recruit_state_full_ele(result);
        recruit_state_less_ele(result);
        recruit_mem_blk(result);
        recruit_mem_elh(result, false);
        recruit_mem_elh(result, true);
        ///// init
        for (ModelProxyBuild* mbp : result){
            ///std::cout << "--------" << std::endl;
            ///std::cout << mbp->get_var_name() << std::endl;
            mbp->proxy_build_init();
        }
        //////////
        for (Module* md : _module->get_user_sub_modules()){
            assert(md != nullptr);
            md->get_sim_engine_ptr()->proxy_build_init();
        }
    }

    std::vector<ModelProxyBuild*> ModuleSimEngine::recruit_for_create_var(){
        std::vector<ModelProxyBuild*> result;
        recruit_state_full_ele(result);
        recruit_state_less_ele(result);
        /** mem block and its subsidaries*/
        recruit_mem_blk(result);
        recruit_mem_elh(result, true);
        recruit_mem_elh(result, false);
        recruit_from_sub_module(result, &ModuleSimEngine::recruit_for_create_var);

        return result;
    }

    std::vector<ModelProxyBuild*> ModuleSimEngine::recruit_for_regis_var(){
        std::vector<ModelProxyBuild*> result;
        /////// recruit sp register first
        recruit_state_full_ele(result);
        recruit_from_vector(result, _module->get_user_wires());
        /** mem block and its subsidaries*/
        recruit_mem_blk(result);
        //// skip mem ele holder recruit_from_mem_elh(result, true);
        //// skip mem ele holder recruit_from_mem_elh(result, false);
        recruit_from_sub_module(result, &ModuleSimEngine::recruit_for_regis_var);
        return result;
    }


    std::vector<ModelProxyBuild*> ModuleSimEngine::recruit_for_main_op_volatile(){
        ///// wire and mem block agent
        std::vector<ModelProxyBuild*> result;
        recruit_state_less_ele(result);
        recruit_mem_elh(result, true);
        recruit_from_sub_module(result, &ModuleSimEngine::recruit_for_main_op_volatile);
        return result;
    }

    std::vector<ModelProxyBuild*> ModuleSimEngine::recruit_for_main_op_non_volatile(){
        return recruit_for_finalize_op();
    }


    std::vector<ModelProxyBuild*> ModuleSimEngine::recruit_for_finalize_op(){
        std::vector<ModelProxyBuild*> result;
        recruit_state_full_ele(result);
        recruit_mem_elh(result, false); /// mem block not include here due to ele handle it by themselve
        recruit_from_sub_module(result, &ModuleSimEngine::recruit_for_finalize_op);
        return result;
    }

    std::vector<LogicSimEngine*> ModuleSimEngine::recruit_for_vcd_var(){
        std::vector<LogicSimEngine*> result;
        for (int sp_idx = 0; sp_idx < SP_CNT_REG; sp_idx++){
            recruit_from_vector(result,
            _module->get_sp_regs(static_cast<SP_REG_TYPE>(sp_idx)));
        }
        recruit_from_vector(result, _module->get_user_regs());
        recruit_from_vector(result, _module->get_user_wires());
        recruit_from_sub_module(result, &ModuleSimEngine::recruit_for_vcd_var);
        return result;
    }

    std::vector<FlowBaseSimEngine*> ModuleSimEngine::recruit_perf(){
        std::vector<FlowBaseSimEngine*> result;
        ///////// recurte flow block in module
        recruit_from_vector(result, _module->get_flow_blocks());
        recruit_from_sub_module(result, &ModuleSimEngine::recruit_perf);
        return result;
    }


    /**
     *
     *
     * MAIN RECRUITER
     *
     */

    void ModuleSimEngine::recruit_state_full_ele
    (std::vector<ModelProxyBuild*>& result){
        recruit_from_sp_reg(result);
        recruit_from_vector(result, _module->get_user_regs());
    }

    void ModuleSimEngine::recruit_state_less_ele
    (std::vector<ModelProxyBuild*>& result){
        recruit_from_vector(result, _module->get_user_wires());
        recruit_from_vector(result, _module->get_user_expressions());
        recruit_from_vector(result, _module->get_user_vals());
        recruit_from_vector(result, _module->get_user_pm_vals());
        recruit_from_vector(result, _module->get_user_nests());
    }

    void ModuleSimEngine::recruit_mem_blk(std::vector<ModelProxyBuild*>& result){
        recruit_from_vector(result, _module->get_user_mem_blks());
    }

    void ModuleSimEngine::recruit_mem_elh(std::vector<ModelProxyBuild*>& result, bool is_read_mode){
        for (MemBlock* mem_block : _module->get_user_mem_blks()){
            for (MemBlockEleHolder* holder_ptr : mem_block->get_mem_block_agents()){
                assert(holder_ptr != nullptr);
                if (holder_ptr->is_read_mode() == is_read_mode){
                    result.push_back(holder_ptr->get_sim_engine_ptr());
                }
            }
        }
    }


    void ModuleSimEngine::recruit_from_sp_reg
    (std::vector<ModelProxyBuild*>& result){
        for (int sp_idx = 0; sp_idx < SP_CNT_REG; sp_idx++){
            recruit_from_vector(result,
            _module->get_sp_regs(static_cast<SP_REG_TYPE>(sp_idx)));
        }
    }


    /**
     *
     *
     * retrieve zone
     *
     ***/


    void ModuleSimEngine::retrieve_init(ProxySimEventBase* sim_event_base){
        /***
         *
         * must same as regist function
         *
         */

        retrieve_init_from_vector(sim_event_base, _module->get_sp_regs(SP_STATE_REG));
        retrieve_init_from_vector(sim_event_base, _module->get_sp_regs(SP_SYNC_REG));
        retrieve_init_from_vector(sim_event_base, _module->get_sp_regs(SP_COND_WAIT_REG));
        retrieve_init_from_vector(sim_event_base, _module->get_sp_regs(SP_CYCLE_WAIT_REG));
        retrieve_init_from_vector(sim_event_base, _module->get_user_regs());
        /////////// wire
        retrieve_init_from_vector(sim_event_base, _module->get_user_wires());
        ////////// memory
        retrieve_init_from_vector(sim_event_base, _module->get_user_mem_blks());

        //////// for flowblock
        retrieve_init_from_vector(sim_event_base, _module->get_flow_blocks());

        ////////// sub_module
        for (Module* sub_module : _module->get_user_sub_modules()){
            sub_module->get_sim_engine_ptr()->retrieve_init(sim_event_base);
        }
    }


    template <typename T>
    void ModuleSimEngine::recruit_from_sub_module(
        std::vector<T*>& result,
        std::vector<T*> (ModuleSimEngine::*func)()){
        for (Module* sub_module : _module->get_user_sub_modules()){
            ModuleSimEngine* mse = sub_module->get_sim_engine_ptr();
            std::vector<T*> sub_result = (mse->*func)();
            append_vector(result, sub_result);
        }
    }

    template <typename S, typename T>
    void ModuleSimEngine::recruit_from_vector(
        std::vector<S*>& result,
        std::vector<T>& ele_vec){
        for (auto ele_ptr : ele_vec){
            assert(ele_ptr != nullptr);
            result.push_back((S*)ele_ptr->get_sim_engine_ptr());
        }
    }

    template <typename T>
    void ModuleSimEngine::retrieve_init_from_vector(
        ProxySimEventBase* sim_event_base,
        std::vector<T*>& ele_vec){
        for (auto ele_ptr : ele_vec){
            assert(ele_ptr != nullptr);
            ele_ptr->get_sim_engine_ptr()->proxy_ret_init(sim_event_base);
        }
    }
}
