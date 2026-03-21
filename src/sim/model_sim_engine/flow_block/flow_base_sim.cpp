//
// Created by tanawin on 17/4/2567.
//

#include "flow_base_sim.h"

#include "sim/controller/sim_controller.h"
#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"
#include "sim/model_sim_engine/base/proxy_event_base.h"
#include "util/str/str_util.h"
#include "model/flow_block/abstract/flow_block__base.h"


namespace kathryn{
    FlowBaseSimEngine::FlowBaseSimEngine(FlowBlockBase* flow_block_base):
        _flowBlockBase(flow_block_base){
        assert(_flowBlockBase != nullptr);
    }

    ValR FlowBaseSimEngine::get_val_rep(){
        return {SIM_VALREP_TYPE_ALL(bit_size_of_ull),
            bit_size_of_ull,
            "PERF_" + _flowBlockBase->get_global_name()};
    }

    ValR FlowBaseSimEngine::get_var_name_cur_status(){
        ValR base = get_val_rep();
        return {SIM_VALREP_TYPE_ALL(1), 1, base.get_data()+"_CURBIT"};
    }

    std::vector<std::string> FlowBaseSimEngine::get_regis_var_name(){
        return {get_val_rep().get_data(), get_var_name_cur_status().get_data()};
    }

    ull FlowBaseSimEngine::get_var_id(){
        return _flowBlockBase->get_global_id();
    }

    SIM_VALREP_TYPE_ALL FlowBaseSimEngine::getValR_Type(){
        return SIM_VALREP_TYPE_ALL(bit_size_of_ull);
    }

    CLOCK_MODE FlowBaseSimEngine::get_clock_mode(){return CM_CLK_UNUSED;}


    void FlowBaseSimEngine::get_recur_var_name(std::vector<std::string>& result){
        result.push_back(get_val_rep().get_data());
        for (FlowBlockBase* fb : _flowBlockBase->get_sub_blocks()){
            FlowBaseSimEngine* sub_block_sim_engine = fb->get_sim_engine_ptr();
            sub_block_sim_engine->get_recur_var_name(result);
        }
        for (FlowBlockBase* fb : _flowBlockBase->get_con_blocks()){
            FlowBaseSimEngine* con_block_sim_engine = fb->get_sim_engine_ptr();
            con_block_sim_engine->get_recur_var_name(result);
        }
    }

    void FlowBaseSimEngine::get_recur_var_name_cur_stsatus(std::vector<std::string>& result){
        result.push_back(get_var_name_cur_status().get_data());
        for (FlowBlockBase* fb : _flowBlockBase->get_sub_blocks()){
            FlowBaseSimEngine* sub_block_sim_engine = fb->get_sim_engine_ptr();
            sub_block_sim_engine->get_recur_var_name_cur_stsatus(result);
        }
        for (FlowBlockBase* fb : _flowBlockBase->get_con_blocks()){
            FlowBaseSimEngine* con_block_sim_engine = fb->get_sim_engine_ptr();
            con_block_sim_engine->get_recur_var_name_cur_stsatus(result);
        }
    }


    void FlowBaseSimEngine::create_global_variable(CbBaseCxx& cb){

        cb.add_st(get_val_rep().build_var(0));
        cb.add_st(get_var_name_cur_status().build_var(0));


        for (FlowBlockBase* fb : _flowBlockBase->get_sub_blocks()){
            FlowBaseSimEngine* sub_block_sim_engine = fb->get_sim_engine_ptr();
            sub_block_sim_engine->create_global_variable(cb);
        }
        for (FlowBlockBase* fb : _flowBlockBase->get_con_blocks()){
            FlowBaseSimEngine* con_block_sim_engine = fb->get_sim_engine_ptr();
            con_block_sim_engine->create_global_variable(cb);
        }
    }

    void FlowBaseSimEngine::create_op(CbBaseCxx& cb){

        cb.add_cm("////////////////////////////////////");
        cb.add_cm(_flowBlockBase->get_global_name());
        ////////////////////////////////////////////////////////////////////////////
        //////////// sub_block build
        ////////////////////////////////////////////////////////////////////////////
        for (FlowBlockBase* fb : _flowBlockBase->get_sub_blocks()){
            FlowBaseSimEngine* sub_block_sim_engine = fb->get_sim_engine_ptr();
            sub_block_sim_engine->create_op(cb.add_sub_block());
        }

        /////////////////////////////////////////////////////////////////////////////
        ///////////// this block purpose
        /////////////////////////////////////////////////////////////////////////////
        cb.add_st(get_var_name_cur_status().eq(ValR(SIM_VALREP_TYPE_ALL(1), 1, "0")).to_string());

        //////////// basic node recruitment
        cb.add_cm("basic node rc");
        for (Node* sys_node : _flowBlockBase->get_sys_nodes()){
            assert(sys_node != nullptr);
            for (CtrlFlowRegBase* state_reg : sys_node->get_cycle_related_reg()){
                if (state_reg != nullptr){
                    ValR state_reg_rep = state_reg->get_sim_engine_ptr()->get_val_rep();
                    ValR check_reg_rep =
                        ValR(SIM_VALREP_TYPE_ALL(state_reg_rep.get_size()), state_reg_rep.get_size(), "0");
                    cb.add_st( get_var_name_cur_status()
                        .eq(get_var_name_cur_status() | (state_reg_rep != check_reg_rep))
                        .to_string());
                }
            }
        }

        ///////////// sub block recruitment
        cb.add_cm("sub block recruitment");
        for (FlowBlockBase* sub_fb : _flowBlockBase->get_sub_blocks()){
            FlowBaseSimEngine* sub_block_sim_engine = sub_fb->get_sim_engine_ptr();
            cb.add_st( get_var_name_cur_status()
                .eq(get_var_name_cur_status() | sub_block_sim_engine->get_var_name_cur_status())
                .to_string()
            );
            for(FlowBlockBase* con_of_sub_fb: sub_fb->get_con_blocks()){
                FlowBaseSimEngine* con_of_sub_fb_sim_engine = con_of_sub_fb->get_sim_engine_ptr();
                cb.add_st( get_var_name_cur_status()
                  .eq(get_var_name_cur_status() | con_of_sub_fb_sim_engine->get_var_name_cur_status())
                  .to_string()
              );
            }

        }
        cb.add_st(
                get_val_rep().eq(
                        (get_val_rep()+
                        get_var_name_cur_status().cast(SIM_VALREP_TYPE_ALL(bit_size_of_ull), bit_size_of_ull)
                        )
                 ).to_string());


        ////////////////////////////////////////////////////////////////////////////
        /////////////// conblock block purpose
        ////////////////////////////////////////////////////////////////////////////

        //////////// do for con block

        for (FlowBlockBase* fb : _flowBlockBase->get_con_blocks()){
            FlowBaseSimEngine* con_block_sim_engine = fb->get_sim_engine_ptr();
            con_block_sim_engine->create_op(cb);
        }

        cb.add_cm("////////////////////////////////////");
    }

    //////////////////// return initiate
    ///
    void FlowBaseSimEngine::proxy_ret_init(ProxySimEventBase* model_sim_event){
        proxy_rep = model_sim_event->get_val_perf(get_val_rep().get_data());
        proxy_rep.set_size(bit_size_of_ull);

        _proxyRepCurBit = model_sim_event->get_val_perf(get_var_name_cur_status().get_data());
        _proxyRepCurBit.set_size(1);
        ///////// subblock init
        for (FlowBlockBase* sub_block : _flowBlockBase->get_sub_blocks()){
            sub_block->get_sim_engine_ptr()->proxy_ret_init(model_sim_event);
        }
        ///////// conblock init
        for (FlowBlockBase* con_block : _flowBlockBase->get_con_blocks()){
            con_block->get_sim_engine_ptr()->proxy_ret_init(model_sim_event);
        }
    }

    ValRepBase& FlowBaseSimEngine::get_proxy_rep(){
        return proxy_rep;
    }

    bool FlowBaseSimEngine::is_block_running(){
        assert(_proxyRepCurBit.is_in_used());
        return _proxyRepCurBit.get_val();
    }
}
