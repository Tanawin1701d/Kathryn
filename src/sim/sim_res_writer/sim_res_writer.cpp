//
// Created by tanawin on 26/1/2567.
//

#include "sim_res_writer.h"

#include "model/hw_component/module/module.h"

#include "sim/model_sim_engine/flow_block/flow_base_sim.h"
#include "model/flow_block/abstract/flow_block__base.h"
#include "util/str/str_util.h"


namespace kathryn{


    /**
     * vcd writer
     *
     ***/

    std::string vcd_sig_type_to_str(VCD_SIG_TYPE st){
        std::string mapper[VST_CNT] = {
                "reg",
                "wire",
                "integer"
        };
        assert(st < VST_CNT);
        return mapper[st];
    }


    VcdWriter::VcdWriter(std::string file_name) : FileWriterBase(file_name) {}

    void VcdWriter::add_new_var(VCD_SIG_TYPE st, std::string name, Slice sl) {
        add_data("$var " +
                vcd_sig_type_to_str(st) + " " +
                std::to_string(sl.get_size()) + " " +
                name + " " +
                name + " $end\n"
                );
    }

    void VcdWriter::add_new_value(const std::string& name, ull val) {
        std::string pre_ret ;
        std::bitset<bit_size_of_ull> binary_representation(val);
        pre_ret += binary_representation.to_string();

        add_new_value(name, pre_ret);
    }

    void VcdWriter::add_new_value(const std::string& name, const std::string& value){
        ///std::cout << "add_dayta"  << name << "    " << value << std::endl;
        add_data(
            "b" + value +
            " " + name  + "\n"
        );

    }

    void VcdWriter::add_new_time_stamp(ull time_stamp){
        add_data("#" + std::to_string(time_stamp) + "\n");
    }

    ////////////////////////////////////// for flow collector

    FlowWriter::FlowWriter(const std::string& file_name)
        : FileWriterBase(file_name){}

    void FlowWriter::start_col_flow_block(FlowBlockBase* fb, int ident){

        /////// this cycle ident
        std::string         indent_str = gen_con_string(' ', ident);
        std::string         show_name  = fb->get_global_name();
        FlowBaseSimEngine*  fbse      = fb->get_sim_engine_ptr();
        ValRepBase&         rep_base   = fbse->get_proxy_rep();
        ull                 cycle     = rep_base.get_val();

        std::string         track_name; ///track name is user optional define tracker_name
        if (fb->is_zep_track_name_set()){
            track_datas.push_back({fb->get_zep_track_name(), cycle});
            track_name = " >>> " + fb->get_zep_track_name();
        }


        add_data(indent_str + show_name + "    " +
            std::to_string(cycle) + track_name + "\n");
        /////// sub block Ident
        for (FlowBlockBase* sub_block: fb->get_sub_blocks()){
            assert(sub_block != nullptr);
            start_col_flow_block(sub_block, ident + SUBFLOW_IDENT);
        }
        if (!fb->get_con_blocks().empty()){
            std::string con_ident_str = gen_con_string(' ', ident + CONFLOW_IDENT);
            add_data(con_ident_str + "-con_block-\n");
            for (FlowBlockBase* con_block: fb->get_con_blocks()){
                assert(con_block != nullptr);
                start_col_flow_block(con_block, ident + CONFLOW_IDENT);
            }
        }
    }

    void FlowWriter::start_col_module(Module* module_to_rec, int ident){
        std::string indent_str = gen_con_string(' ',ident);
        std::string show_name = "Module " + module_to_rec->get_global_name();
        add_data(indent_str + show_name + "\n");
        for (FlowBlockBase* fb: module_to_rec->get_flow_blocks()){
            start_col_flow_block(fb, ident + SUBFLOW_IDENT);
        }
        if (!module_to_rec->get_user_sub_modules().empty()){
            std::string sub_indent_str = gen_con_string(' ', ident + SUBMOD_IDENT);
            add_data(sub_indent_str + "-sub_mod-\n");
            for (Module* sub_module: module_to_rec->get_user_sub_modules()){
                start_col_module(sub_module, ident + SUBMOD_IDENT);
            }
        }
    }

    void FlowWriter::start_write_summary(){
        int max_length = 0;
        ////find max length to make the match column
        for (const auto&[key, value]: track_datas){
            max_length = std::max(static_cast<int>(key.size()), max_length);
        }

        for (const auto&[key, value]: track_datas){
            add_data(key + gen_con_string(' ', max_length+1-key.size())
                        + " : " + std::to_string(value) + "\n");
        }
    }

    void FlowWriter::start_write_data(){
        start_col_module(_topRecMod, 0);
        add_data("\n\n\n\n\n ------------- User Defined Summary -------------\n");
        start_write_summary();
    }

    void FlowWriter::init(Module* top_module){
        assert(top_module != nullptr);
        _topRecMod = top_module;
    }

}