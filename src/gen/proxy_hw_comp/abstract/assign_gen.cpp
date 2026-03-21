//
// Created by tanawin on 22/6/2024.
//

#include "assign_gen.h"

#include "gen/proxy_hw_comp/module/module_gen.h"

namespace kathryn{


    AssignGenBase::~AssignGenBase(){
        translated_update_pool.clean();
    }


    void AssignGenBase::route_dep(){

        assert(_asb != nullptr);
        _asb->sort_up_event_by_priority();
        /** copy the translated_update_pool*/
        translated_update_pool = _asb->get_update_meta().clone();
        /** try to reroute the update Event*/
        for(UpdateEventBase* ueb: translated_update_pool.events){
            UEBaseGenEngine* gen_engine = ueb->create_gen_engine();
            gen_engine->reroute(_mdGenMaster);
            delete gen_engine;
        }
    }

    std::pair<Verilog_SEN_TYPE, std::string>
    AssignGenBase::get_clock_sen_info(){

        assert(_asb != nullptr);
        CLOCK_MODE clk_mode = _asb->get_update_meta().get_clock_mode();
        switch (clk_mode){
            case CM_POSEDGE: return std::make_pair(VLST_POSEDGE, "clk");
            case CM_NEGEDGE: return std::make_pair(VLST_NEGEDGE, "clk");
            default: return std::make_pair(VLST_ALWAYS, "*");
        }

    }



    std::string AssignGenBase::assign_op_with_sole_condition(){

        ////// if there is no update event skip it
        if (translated_update_pool.is_empty()){
            return "";
        }

        ////// sensitivity list have to be the same in the system
        auto [sen_type, sen_name] = get_clock_sen_info();

        std::string ret_str;
        CbAlwaysVerilog cb_aw(sen_type, sen_name);

        for (UpdateEventBase* ueb: translated_update_pool.events){
            UEBaseGenEngine* gen_engine = ueb->create_gen_engine();
            gen_engine->gen_ass(cb_aw, this);
            delete gen_engine;
        }

        ret_str = cb_aw.to_string(4);
        return ret_str;

    }



    std::string AssignGenBase::assign_op_base(){
        return assign_op_with_sole_condition();

    }

    std::string AssignGenBase::assignment_line(Slice des_slice, Operable* src_update_value, bool is_delayed_asm){
        assert(src_update_value != nullptr);
        std::string asm_opr = is_delayed_asm ? " <= " : " = ";
        return get_opr(des_slice) + asm_opr + get_opr_str_from_opr_and_shink_msb(src_update_value, des_slice.get_size());
    }
}
