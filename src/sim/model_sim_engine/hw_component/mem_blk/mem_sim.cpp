//
// Created by tanawin on 31/5/2024.
//

#include "mem_sim.h"
#include "model/hw_component/mem_block/mem_block.h"
#include "sim/controller/sim_controller.h"



namespace kathryn{
    MemSimEngine::MemSimEngine(MemBlock* master):
        _master(master){ assert(master != nullptr); }

    ValR MemSimEngine::get_val_rep(){
        std::string     name = _master->get_global_name();
        int             size = _master->get_width_size();

        return {SIM_VALREP_TYPE_ALL(size), size, name};
    }

    std::vector<std::string> MemSimEngine::get_regis_var_name(){
        return {get_val_rep().get_data()};
    }


    ull MemSimEngine::get_var_id(){ return _master->get_global_id(); }

    SIM_VALREP_TYPE_ALL MemSimEngine::getValR_Type(){
        return SIM_VALREP_TYPE_ALL(_master->get_width_size());
    }

    void MemSimEngine::mark_sv(const std::string& str){
        markSV_base(str);
    }

    void MemSimEngine::create_global_variable(CbBaseCxx& cb){
        ull depth = _master->get_depth_size();
        cb.add_st(get_val_rep().build_mem_var(depth));
    }

    void MemSimEngine::create_user_mark_value(CbBaseCxx& cb){
        if (is_mark_sv){
            cb.add_st(get_val_rep().build_mem_var_ptr(markSV_key));
        }
    }


    ///////   return value

    void MemSimEngine::proxy_ret_init(ProxySimEventBase* model_sim_event){
        proxy_rep = model_sim_event->get_val(get_val_rep().get_data());
        proxy_rep.set_size(_master->get_width_size());
        if (getValR_Type().type == SVT_U64M){
            assert(getValR_Type().sub_type > 0);
            proxy_rep.set_contin_length(getValR_Type().sub_type);
        }
    }

    ValRepBase& MemSimEngine::get_proxy_rep(){
        assert(proxy_rep.is_in_used());
        return proxy_rep;
    }
}
