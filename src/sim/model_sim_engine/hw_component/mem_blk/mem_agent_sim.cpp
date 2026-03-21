//
// Created by tanawin on 18/7/2024.
//

#include "mem_agent_sim.h"

#include "model/hw_component/mem_block/mem_block_agent.h"
#include "model/hw_component/mem_block/mem_block.h"
#include "sim/model_sim_engine/hw_component/abstract/gen_helper.h"

namespace kathryn{



    /**
     *
     * MemEleSimEngine
     *
     * */

    MemEleHolderSimEngine::MemEleHolderSimEngine(MemBlockEleHolder* master):
    LogicSimEngine(master, master, master, VST_WIRE, false, 0),
    _master(master){
        assert(master != nullptr);
    }

    void MemEleHolderSimEngine::proxy_build_init(){
        assert(_master->_indexer != nullptr);
        dep.push_back(_master->_indexer->get_logic_sim_engine_from_opr_ptr());

        ///// write mode
        if (_master->is_write_mode()){
            LogicSimEngine::proxy_build_init();
        }
    }


    void MemEleHolderSimEngine::create_global_variable(CbBaseCxx& cb){

        if(_master->is_write_mode()){

            cb.add_st(get_val_rep    ().build_var(_initVal) , false);
            cb.add_st(get_is_set_var  ().build_var(0)   , false);
            cb.add_st(get_indexer_var().build_var(0)   , true);
        }

        if (_reqGlobDec && _master->is_read_mode()){
            cb.add_st(get_val_rep().build_var(_initVal));
        }
    }

    void MemEleHolderSimEngine::create_local_variable(CbBaseCxx& cb){
        if((!_reqGlobDec) && _master->is_read_mode()){
            cb.add_st(get_val_rep().build_var(_initVal));
        }
    }

    void MemEleHolderSimEngine::create_op(CbBaseCxx& cb){
        _asb->sort_up_event_by_priority();
        if (_master->is_write_mode()){ ///// write mode
            return create_op_write_mode(cb);
        }

        if (_master->is_read_mode()){
            return create_op_read_mode(cb);
        }

        assert(false);

    }


    ValR MemEleHolderSimEngine::get_is_set_var(){
        std::string name = get_val_rep()._data + IS_SET_SUFFIX;
        return {SIM_VALREP_TYPE_ALL(1), 1, name};
    }

    ValR MemEleHolderSimEngine::get_indexer_var(){
        std::string name = get_val_rep()._data + INDEXER_SUFFIX;
        int         size = _master->get_exact_index_size();
        return {SIM_VALREP_TYPE_ALL(size), size, name};
    }


    void MemEleHolderSimEngine::create_op_read_mode(CbBaseCxx& cb){
        ///
        /////// read mode
        ///
        cb.add_cm(_ident->get_global_name() + "  read_mode");
        ////// we are so sure that it is the same as the width
        ValR mem_val_r = _master->_master->get_sim_engine_ptr()->get_val_rep();
        ////////// we must fix indexer type to match main memory
        ValR indexer = get_sliced_src_opr_from_opr(_master->_indexer,get_indexer_var()._valType);

        cb.add_st(get_val_rep().eq(mem_val_r.index(indexer))
        .to_string());
    }

    void MemEleHolderSimEngine::create_op_write_mode(CbBaseCxx& cb){

        ValR setter_eq  = get_is_set_var().eq(ValR(SIM_VALREP_TYPE_ALL(1), 1, "1"));
        ValR indexer   = get_sliced_src_opr_from_opr(_master->_indexer,get_indexer_var()._valType);
        ////// index and push to local variable
        ValR indexer_eq = get_indexer_var().eq(indexer);
        std::string aux_ass_val = setter_eq.to_string() + "; " + indexer_eq.to_string() + ";";

        ///////// build string
        cb.add_cm(_ident->get_global_name());
        //assert(_asb->check_des_is_fully_assign_and_equal());
        create_op_with_sole_condition(cb, aux_ass_val);
    }


    void MemEleHolderSimEngine::create_op_end_cycle(CbBaseCxx& cb){

        if (_master->is_write_mode()){
            cb.add_cm(_ident->get_global_name());

            CbIfCxx&  if_block = cb.add_if(get_is_set_var().to_string());
            ValR mem_blk_val_r = _master->_master->get_sim_engine_ptr()->get_val_rep();

            ValR ass_eq = mem_blk_val_r.index(get_indexer_var()).eq(get_val_rep());
            ValR rst_set_flag = get_is_set_var().eq(ValR(SIM_VALREP_TYPE_ALL(1), 1, "0"));

            ///////////// add value
            if_block.add_st(ass_eq.to_string());
            ///////////// reset is set
            if_block.add_st(rst_set_flag.to_string());
        }

    }
}