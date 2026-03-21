//
// Created by tanawin on 27/11/25.
//

#include "gen/proxy_hw_comp/abstract/update_event.h"
#include "gen/proxy_hw_comp/module/module_gen.h"


namespace kathryn{
    /**
     * gen base
     */

    UEBaseGenEngine::~UEBaseGenEngine(){
        for (UEBaseGenEngine* gen_engine: sub_engine){
            delete gen_engine;
        }
    }

    Operable* UEBaseGenEngine::reroute_base(Operable* src_opr, ModuleGen* md_gen){
        if (src_opr == nullptr){
            return nullptr;
        }
        return md_gen->route_src_opr_to_this_module(src_opr);
    }

    void UEBaseGenEngine::reroute_and_replace(Operable*& src_opr, ModuleGen* md_gen){
        src_opr = reroute_base(src_opr, md_gen);
    }

    /**
     * basic
     */
    bool UEBasicGenEngine::validate_assign_sensivity() const{
        return ((master->_clkMode == CM_POSEDGE)  ||
               (master->_clkMode == CM_NEGEDGE)  ||
               (master->_clkMode == CM_CLK_FREE)
               );
    }


    void UEBasicGenEngine::reroute(ModuleGen* md_gen){
        reroute_and_replace(master->_value, md_gen);
    }

    void UEBasicGenEngine::gen_ass(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen){
        assert(validate_assign_sensivity());
        bool is_clock_sen = (master->_clkMode == CM_POSEDGE)  || (master->_clkMode == CM_NEGEDGE);
        std::string ass_str = assign_gen->assignment_line(
            master->_desSlice, master->_value, is_clock_sen
        );
        cb_ver.add_st(ass_str);
    }

    void UEBasicGenEngine::gen_basic_connect(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen){

        std::string ass_str = "assign " + assign_gen->get_opr() +
                             " = "     +  assign_gen->get_opr_str_from_opr(master->_value);
        cb_ver.add_st(ass_str);
    }

    /**
     * grp
     */
    void UEGrpGenEngine::reroute(ModuleGen* md_gen){
        for (UpdateEventBase* ueb: master->sub_stmts){
            UEBaseGenEngine* gen_engine = ueb->create_gen_engine();
            gen_engine->reroute(md_gen);
            sub_engine.push_back(gen_engine);
        }
    }

    void UEGrpGenEngine::gen_ass(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen){

        for(UpdateEventBase* ueb: master->sub_stmts){
            UEBaseGenEngine* gen_engine = ueb->create_gen_engine();
            sub_engine.push_back(gen_engine);
            gen_engine->gen_ass(cb_ver, assign_gen);
        }

    }

    /**
     * cond
     */
    void UECondGenEngine::reroute(ModuleGen* md_gen){
        for (int idx = 0; idx < master->conditions.size(); idx++){
            reroute_and_replace(master->conditions[idx], md_gen);
        }
        for (UpdateEventBase* ueb: master->sub_stmts){
            UEBaseGenEngine* gen_engine = ueb->create_gen_engine();
            sub_engine.push_back(gen_engine);
            gen_engine->reroute(md_gen);
        }
    }

    void UECondGenEngine::gen_ass(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen){

        CbIfVerilog* first_if_stmt = nullptr;
        assert(master->conditions.size() == master->sub_stmts.size());
        for (int i = 0; i < master->conditions.size(); ++i){
            Operable* condition = master->conditions[i];
            UpdateEventBase* ueb = master->sub_stmts[i];
            std::string cond_str = "1'b1";
            if (condition != nullptr){
                cond_str = assign_gen->get_opr_str_from_opr(condition);
            }
            UEBaseGenEngine* gen_engine = ueb->create_gen_engine();

            ///// creating the block
            CbIfVerilog* cur_block = nullptr;

            if (first_if_stmt == nullptr){
                first_if_stmt = &cb_ver.add_if(cond_str);
                cur_block    = first_if_stmt;
            }else{
                cur_block    = &first_if_stmt->add_elif(cond_str);
            }
            gen_engine->gen_ass(*cur_block, assign_gen);
            sub_engine.push_back(gen_engine);

        }

    }

    void UECondGenEngine::gen_basic_connect(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen){
        assert(master->conditions.size() == 1);
        assert(master->sub_stmts.size() == 1);
        assert(master->conditions[0] == nullptr);
        auto* gen_engine = master->sub_stmts[0]->create_gen_engine();
        gen_engine->gen_basic_connect(cb_ver, assign_gen);
        sub_engine.push_back(gen_engine);

    }


    /**
     *
     * switch
     */
    void UESwitchGenEngine::reroute(ModuleGen* md_gen){
        Operable*& ident_ref = master->get_state_ident_ref();
        reroute_and_replace(ident_ref, md_gen);
        for (UpdateEventBase* ueb: master->sub_stmts){
            if (ueb != nullptr){
                UEBaseGenEngine* gen_engine = ueb->create_gen_engine();
                gen_engine->reroute(md_gen);
                sub_engine.push_back(gen_engine);
            }
        }
    }

    void UESwitchGenEngine::gen_ass(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen){

        std::string state_ident = assign_gen->get_opr_str_from_opr(master->get_state_ident_ptr());
         CbSwitchVerilog* cb_ver_switch = &cb_ver.add_switch(state_ident);

        for (int idx = 0; idx < master->get_match_num(); idx++){

            int              match_idx    = master->get_sub_stmt_match_idxs(idx);
            CbBaseVerilog* case_block = &cb_ver_switch->add_case(match_idx);

            UpdateEventBase* ueb = master->get_sub_stmts_ptr(idx);
            if (ueb != nullptr){
                UEBaseGenEngine* gen_engine = ueb->create_gen_engine();
                gen_engine->gen_ass(*case_block, assign_gen);
                sub_engine.push_back(gen_engine);
            }

        }


    }

}