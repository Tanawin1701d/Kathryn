//
// Created by tanawin on 27/11/25.
//

#include "sim/model_sim_engine/hw_component/abstract/update_event.h"
#include "model/hw_component/abstract/update_event.h"
#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"

#include "gen_helper.h"


namespace kathryn{

    /**
     * update event base
     *
     */

    UpdateEventBaseSimEngine::~UpdateEventBaseSimEngine(){
        for (UpdateEventBaseSimEngine* sim_engine: sub_engine){
            delete sim_engine;
        }
    }

    /**
     * update event basic
     *
     */

    void UpdateEventBasicSimEngine::create_sim_op(CbBaseCxx& cb,
                                        LogicSimEngine& lse,
                                        const std::string& aux_ass_str){

        std::string ass_str = lse.gen_assign_a_eq_b(master->_desSlice, lse.is_temp_req(), master->_value);
        cb.add_st(ass_str);
        if (!aux_ass_str.empty()){
            cb.add_st(aux_ass_str); //// make it more beutiful
        }
    }

    /**
     * update grp event
     *
     ***/
    void UpdateEventGrpSimEngine::create_sim_op(CbBaseCxx& cb,
                                     LogicSimEngine& lse,
                                     const std::string& aux_ass_str){

        for(UpdateEventBase* ueb: master->sub_stmts){

            UpdateEventBaseSimEngine* sim_engine = ueb->create_sim_event();
            sub_engine.push_back(sim_engine);
            sim_engine->create_sim_op(cb, lse, aux_ass_str);
        }

    }

    /**
     * update cond event
     *
     */

    void UpdateEventCondSimEngine::create_sim_op(CbBaseCxx& cb,
                                      LogicSimEngine& logic_sim_engine,
                                      const std::string& aux_ass_str){

        CbIfCxx* first_if_stmt = nullptr;
        assert(master->conditions.size() == master->sub_stmts.size());
        for (int i = 0; i < master->conditions.size(); ++i){
            Operable* condition  = master->conditions[i];
            UpdateEventBase* ueb = master->sub_stmts[i];
            std::string cond_str  = "true";
            if (condition != nullptr){
                cond_str = get_sliced_src_opr_from_opr(condition).to_string();
            }
            UpdateEventBaseSimEngine* sim_engine = ueb->create_sim_event();

            ///// creating the block
            CbIfCxx* cur_write_block = nullptr;

            if (first_if_stmt == nullptr){
                first_if_stmt = &cb.add_if(cond_str);
                cur_write_block = first_if_stmt;
            }else{
                cur_write_block = &first_if_stmt->add_elif(cond_str);

            }
            sim_engine->create_sim_op(*cur_write_block, logic_sim_engine, aux_ass_str);
            //sim_engine->create_sim_op(*first_if_stmt, logic_sim_engine, aux_ass_str);
            sub_engine.push_back(sim_engine);

        }
    }

    /**
     * update state event
     */

    void UpdateEventSwitchSimEngine::create_sim_op(CbBaseCxx& cb,
                                      LogicSimEngine& logic_sim_engine,
                                      const std::string& aux_ass_str){
//assert(false);
        //////// get switch identifier
        Operable* state_iden  = master->state_iden;
        std::string ident_str = get_sliced_src_opr_from_opr(state_iden).to_string();
        //////// build add ident to cxx block
        CbSwitchCxx* switch_cxx = &cb.add_switch(ident_str);

        for (int idx = 0; idx < master->get_match_num(); idx++){

            ///// build case
            int                       match_idx  = master->get_sub_stmt_match_idxs(idx);
            CbBaseCxx* match_work_block = &switch_cxx->add_case(match_idx);
            ///// update event case
            UpdateEventBase*          ueb       = master->get_sub_stmts_ptr(idx);
            if (ueb != nullptr){
                UpdateEventBaseSimEngine* sim_engine = ueb->create_sim_event();
                sim_engine->create_sim_op(*match_work_block, logic_sim_engine, aux_ass_str);
                sub_engine.push_back(sim_engine);
            }


        }



    }
}
