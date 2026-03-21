//
// Created by tanawin on 31/5/2024.
//

#include "logic_sim_engine.h"

#include "gen_helper.h"
#include "sim/controller/sim_controller.h"


namespace kathryn{
    LogicSimEngine::LogicSimEngine(Assignable* asb, Identifiable* ident, Operable* opr1,
                                   VCD_SIG_TYPE sig_type, bool is_temp_req,
                                   ull init_val):
        _asb(asb),
        _ident(ident),
        _opr(opr1),
        _vcdSigType(sig_type),
        _isTempReq(is_temp_req),
        _initVal(init_val){
        assert(asb != nullptr);
        assert(ident != nullptr);
        //assert(_vcdSigType != VCD_SIG_TYPE::VST_DUMMY);
        ///////// fill asb  to system to support topology sort
    }

    void LogicSimEngine::create_op_with_sole_condition(CbBaseCxx& cb,
                                                   const std::string& aux_ass_str){

        for (UpdateEventBase* update_event : _asb->get_update_meta().get_update_event_ref()){

            cb.add_cm(get_val_rep().to_string() + ": priority " + std::to_string(update_event->get_priority()));
            UpdateEventBaseSimEngine* ue_sim_engine = update_event->create_sim_event();
            ue_sim_engine->create_sim_op(cb, *this, aux_ass_str);
            delete ue_sim_engine;

        }
    }

    std::string LogicSimEngine::gen_assign_a_eq_b(Slice des_slice, bool is_des_temp,
                                              Operable* src_opr){
        if (_ident->get_global_id() == 103){
            std::cout << "got 103" << std::endl;
        }
        assert(src_opr != nullptr);
        assert(des_slice.stop <= _asb->get_assign_slice().stop);
        ////// src operand
        Slice           src_slice          = src_opr->get_operable_slice();
        Slice           base_src_slice      = src_opr->get_exact_operable().get_operable_slice();
        ValR            src_var            = get_src_opr_from_opr(src_opr);

        assert(src_slice.get_size() >= des_slice.get_size());
        ////// des operand
        Slice       base_des_slice = _asb->get_assign_slice();
        ValR        des_var       = is_des_temp ? get_temp_val_rep() : get_val_rep();

        if ((des_slice == base_des_slice) &&
            (src_slice == base_src_slice) &&
            (src_slice == des_slice)){
            ///////// optimize
            return des_var.eq(src_var).to_string();
        }


        std::string ret;
        std::string des_a_str = std::to_string(des_slice.start);
        std::string des_b_str = std::to_string(des_slice.stop);
        /////////////////////// clear old data
        ret += des_var.eq(des_var.clear(des_slice)).to_string() + ";\n";
        ret += "        ";
        ////////////////////// create new data

        ValR fill_val = get_sliced_and_shift_src_opr_from_opr(src_opr, des_slice, getValR_Type());
        ret += des_var
        .eq(des_var.partial_or(fill_val))
        .to_string() + ";";

        return ret;
    }

    ValR LogicSimEngine::gen_src_opr(){
        return get_val_rep();
    }

    ValR LogicSimEngine::gen_sliced_opr_to(Slice src_slice, SIM_VALREP_TYPE_ALL des_field){
        assert(src_slice.check_valid_slice() &&
            (src_slice.stop <= _asb->get_assign_slice().stop));
        ////// it will automatic shift to 0 index
        ////// des operand
        Slice base_src_slice = _asb->get_assign_slice();
        SIM_VALREP_TYPE_ALL base_type = getValR_Type();

        if ( (base_src_slice == src_slice) &&
             (base_type     == des_field)
        ){
            return get_val_rep();
        }

        std::string a_str = std::to_string(src_slice.start);
        std::string b_str = std::to_string(src_slice.stop);
        return get_val_rep()
                .slice(src_slice)
                .cast(des_field, src_slice.get_size());

    }

    ValR LogicSimEngine::gen_sliced_opr_and_shift(Slice des_slice, Slice src_slice, SIM_VALREP_TYPE_ALL des_field){
        assert(src_slice.check_valid_slice() &&
            (src_slice.stop <= _asb->get_assign_slice().stop));
        assert(des_slice.get_size() <= src_slice.get_size());

        Slice base_src_slice = _asb->get_assign_slice();
        SIM_VALREP_TYPE_ALL base_type = getValR_Type();

        if ((base_src_slice == src_slice) &&
            (base_src_slice == des_slice) &&
            (base_type     == des_field)
        ){ return get_val_rep();}
        ///// may be our destination may have shorter bit width
        int actual_size = std::min(src_slice.get_size(), des_slice.get_size());
        return gen_sliced_opr_to({src_slice.start, src_slice.start + actual_size}, des_field).
                shift(des_slice.start);
    }


    void LogicSimEngine::proxy_build_init(){
        //// std::cout << _ident->get_var_name() << std::endl;
        _asb->sort_up_event_by_priority();

        for (UpdateEventBase* update_event : _asb->get_update_meta().get_update_event_ref()){
            std::vector<Operable*> dep_opr;
            update_event->get_dep(dep_opr);
            for (Operable* opr1 : dep_opr){
                assert(opr1 != nullptr);
                dep.push_back(opr1->get_logic_sim_engine_from_opr_ptr());
            }
        }
    }

    ValR LogicSimEngine::get_val_rep(){
        std::string name = _ident->get_global_name() +
            (_ident->is_user_var() ? "_USER_" + _ident->get_var_name() : "_SYS");

        int size = _asb->get_assign_slice().get_size();
        SIM_VALREP_TYPE_ALL val_type = getValR_Type();
        return {val_type, size, name};
    }

    std::vector<std::string> LogicSimEngine::get_regis_var_name(){
        return {get_val_rep().get_data()};
    }

    ValR LogicSimEngine::get_temp_val_rep(){
        ValR base = get_val_rep();
        base.set_data(base.get_data() + TEMP_VAR_SUFFIX);
        return base;
    }

    SIM_VALREP_TYPE_ALL LogicSimEngine::getValR_Type(){
        return SIM_VALREP_TYPE_ALL(_asb->get_assign_slice().get_size());
    }

    CLOCK_MODE LogicSimEngine::get_clock_mode(){
        assert(_asb != nullptr);
        return _asb->get_update_meta().get_clock_mode();
    }


    void LogicSimEngine::create_global_variable(CbBaseCxx& cb){
        ////////"; will be auto add"

        cb.add_st( get_val_rep().build_var(_initVal), !_isTempReq);
        if (_isTempReq){
            cb.add_st(get_temp_val_rep().build_var(_initVal));
        }
    }


    void LogicSimEngine::create_op(CbBaseCxx& cb){
        ///////// build string
        cb.add_cm(_ident->get_global_name());

        _asb->sort_up_event_by_priority();
        if (_isTempReq){
            cb.add_st( //// build temp variable first
                get_temp_val_rep().eq(get_val_rep()).to_string()
            );
        }


        create_op_with_sole_condition(cb);

        // if (_asb->check_des_is_fully_assign_and_equal()){
        //     gen_op_with_chain_condition(cb);
        // }else{
        //     create_op_with_sole_condition(cb);
        // }
    }

    void LogicSimEngine::create_op_end_cycle2(CbBaseCxx& cb){
        if (_isTempReq){
            cb.add_st(
                get_val_rep().eq(get_temp_val_rep()).to_string()
            );
        }
    }

    void LogicSimEngine::create_user_mark_value(CbBaseCxx& cb){
        if (is_mark_sv){
            cb.add_st( get_val_rep().build_var_ref(markSV_key));
        }
    }

    ///////////////////// proxy_ret_init
    ///
    void LogicSimEngine::proxy_ret_init(ProxySimEventBase* model_sim_event){
        proxy_rep = model_sim_event->get_val(get_val_rep().to_string());
        proxy_rep.set_size(_asb->get_assign_slice().get_size());
        if (getValR_Type().type == SVT_U64M){
            assert(getValR_Type().sub_type > 0);
            proxy_rep.set_contin_length(getValR_Type().sub_type);
        }
        ///// we cache it in operable
        _opr->init_val_rep(proxy_rep);
    }



    ValRepBase& LogicSimEngine::get_proxy_rep(){
        mf_assert(proxy_rep.is_in_used(), "you might access the element that have not been tied with "
                 "registeration of proxy sim manager");
        return proxy_rep;
    }




   // void LogicSimEngine::gen_op_with_chain_condition(CbBaseCxx& cb, const std::string& aux_ass_str){

           // CbIfCxx* first_if_statement = nullptr;
           //
           //  int idx = 0;
           //  int max_update_event = static_cast<int>(_asb->get_update_meta().size());
           //  std::vector<UpdateEvent*> reversed_update_events = _asb->get_update_meta();
           //  std::reverse(reversed_update_events.begin(), reversed_update_events.end());
           //
           //  while (idx < max_update_event){
           //
           //      std::vector<UpdateEvent*> update_event_grp;
           //      update_event_grp.push_back(reversed_update_events[idx]);
           //      for (idx = idx + 1; idx < max_update_event; idx++){
           //          UpdateEvent& cur_update_event = *reversed_update_events[idx];
           //          if (cur_update_event.priority == update_event_grp[0]->priority){
           //              if (cur_update_event.src_update_value->is_const_opr() &&
           //                  update_event_grp[0]->src_update_value->is_const_opr() &&
           //                  (cur_update_event.src_update_value->get_const_opr() ==
           //                      update_event_grp[0]->src_update_value->get_const_opr())
           //              ){
           //                  update_event_grp.push_back(reversed_update_events[idx]);
           //                  continue; ///// grp update_event for const value
           //              }
           //              if (cur_update_event.src_update_value ==
           //                  update_event_grp[0]->src_update_value){
           //                  update_event_grp.push_back(reversed_update_events[idx]);
           //                  continue; ///// grp update_event for other value
           //              }
           //          }
           //          break;
           //      }
           //
           //      assert(update_event_grp[0]->src_update_value->get_operable_slice().get_size() >=
           //          update_event_grp[0]->des_update_slice.get_size());
           //
           //
           //      std::string con_str;
           //
           //      for (UpdateEvent* update_event : update_event_grp){
           //          bool is_sub_con_occur = false;
           //          con_str += "(";
           //          if (update_event->src_update_state != nullptr){
           //              con_str += get_sliced_src_opr_from_opr(update_event->src_update_state).to_string();
           //              is_sub_con_occur = true;
           //          }
           //
           //          if (update_event->src_update_condition != nullptr){
           //              if (is_sub_con_occur){
           //                  con_str += " && ";
           //              }
           //              con_str += get_sliced_src_opr_from_opr(update_event->src_update_condition).to_string();
           //              is_sub_con_occur = true;
           //          }
           //
           //          if (!is_sub_con_occur){
           //              con_str += "true";
           //          }
           //          con_str += ")";
           //          if ((update_event) != (*update_event_grp.rbegin())){
           //              con_str += " || ";
           //          }
           //      }
           //
           //
           //      CbIfCxx* cur_if_statement = nullptr;
           //      if (first_if_statement == nullptr){
           //          first_if_statement = &cb.add_if(con_str);
           //          cur_if_statement   = first_if_statement;
           //      }else{
           //          cur_if_statement   = &first_if_statement->add_elif(con_str);
           //      }
           //
           //
           //      cur_if_statement->add_st(gen_assign_a_eq_b(update_event_grp[0]->des_update_slice, _isTempReq,
           //                            update_event_grp[0]->src_update_value));
           //      if (!aux_ass_str.empty()){
           //          cur_if_statement->add_st(aux_ass_str);
           //      }
           //  }

    //    }
}