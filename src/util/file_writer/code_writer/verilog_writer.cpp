//
// Created by tanawin on 27/11/25.
//

#include "verilog_writer.h"

#include "utility"
#include "util/str/str_util.h"

namespace kathryn{
    /**
     *  CbBaseVerilog
     */
    CbIfVerilog& CbBaseVerilog::add_if(std::string condition){
        auto* if_block = new CbIfVerilog(false,std::move(condition));
        append_sub_block(if_block);
        return *if_block;
    }

    CbBaseVerilog& CbBaseVerilog::add_sub_block(){
        auto* sub_block = new CbBaseVerilog();
        append_sub_block(sub_block);
        return *sub_block;
    }

    CbAlwaysVerilog& CbBaseVerilog::add_always(Verilog_SEN_TYPE sen_type, std::string sen_name){
        auto* always_block = new CbAlwaysVerilog(sen_type,std::move(sen_name));
        append_sub_block(always_block);
        return *always_block;
    }

    CbSwitchVerilog& CbBaseVerilog::add_switch(std::string switch_ident){
        auto* switch_block = new CbSwitchVerilog(std::move(switch_ident));
        append_sub_block(switch_block);
        return *switch_block;
    }


    std::string CbBaseVerilog::to_string(int ident){

        //////// the local idx of eachtype

        /////////// sb for subblock index
        /////////// st for statement index
        int next_sb_check_idx = 0;
        int next_st_check_idx = 0;

        std::string pre_ret;
        std::string ident_val = gen_con_string(' ', ident);

        for (int main_order = 0; main_order < last_order; main_order++){

            ///////// we have to pool it inorder no mater it is subblock or statement

            if (next_st_check_idx < _codeSt.size() &&
                _codeSt[next_st_check_idx].order == main_order){
                if (_codeSt[next_st_check_idx].is_comment){pre_ret += "////";}
                pre_ret += ident_val + _codeSt[next_st_check_idx].st + ";";
                if (_codeSt[next_st_check_idx].ln){pre_ret += "\n";}
                next_st_check_idx++;
                continue;
            }

            if (next_sb_check_idx < _sbOrder.size() &&
                _sbOrder[next_sb_check_idx] == main_order){
                pre_ret += _subBlocks[next_sb_check_idx]->to_string(ident + Verilog_IDENT);
                next_sb_check_idx++;
                continue;
            }
            assert(false);
        }
        return pre_ret;
    }

    /**
     *
     * CbIfVerilog
     *
     */

    CbIfVerilog::CbIfVerilog(bool is_sub_chain, std::string condtion):
    _markAsSubChain(is_sub_chain),
    _cond(std::move(condtion)){}

    CbIfVerilog::~CbIfVerilog(){
        for (CbIfVerilog* cont_block: _contBlock){
            delete cont_block;
        }
    }

    CbIfVerilog& CbIfVerilog::add_elif(std::string condition){
        auto* elif_block = new CbIfVerilog(true, std::move(condition));
        _contBlock.push_back(elif_block);
        return *elif_block;
    }

    std::string CbIfVerilog::to_string(int ident){
        std::string pre_ret;
        std::string indent_val = gen_con_string(' ', ident);

        if (!_markAsSubChain){
            pre_ret += indent_val + "if(" + _cond + ")";
        }else if (!_cond.empty()){
            pre_ret += "else if(" + _cond + ")";
        }else{
            pre_ret += "else";
        }

        pre_ret += "begin\n";

        pre_ret += CbBaseVerilog::to_string(ident + Verilog_IDENT);

        pre_ret += indent_val + "end ";

        for(CbIfVerilog* cont_block: _contBlock){
            pre_ret += cont_block->to_string(ident);
        }
        if (!_markAsSubChain){
            pre_ret += "\n";
        }
        return pre_ret;
    }

    /**
     *
     * CbAlwaysVerilog
     *
     */

    CbAlwaysVerilog::CbAlwaysVerilog(Verilog_SEN_TYPE sen_type, std::string  sen_name):
    _senType(sen_type),
    _senName(std::move(sen_name)){}

    std::string CbAlwaysVerilog::to_string(int ident){

        assert(_senType >= 0 && _senType < VLST_CNT);

        std::string pre_ret;
        std::string indent_val = gen_con_string(' ', ident);

        std::string sen_name = (_senType == VLST_ALWAYS) ? "*" : (Verilog_SEN_TYPE_STR[_senType] + " " +_senName);

        pre_ret += indent_val + "always @(" + sen_name + " ) begin\n";
        pre_ret += CbBaseVerilog::to_string(ident + Verilog_IDENT);
        pre_ret += indent_val + "end";

        return pre_ret;

    }

    /**
     *
     * CbSwitchVerilog
     *
     */

    CbBaseVerilog& CbSwitchVerilog::add_case(int case_val){
        if (case_val == -1){
            assert(!is_default_occure);
            is_default_occure = true;
        }
        _caseIdents.push_back(case_val);
        return CbBaseVerilog::add_sub_block();
    }

    std::string CbSwitchVerilog::to_string(int ident){
        std::string pre_ret;
        std::string indent_val = gen_con_string(' ', ident);
        std::string indent_val_inside = gen_con_string(' ', ident + Verilog_IDENT);

        pre_ret += indent_val + "case(" + _switchIdent + ")\n";

        assert(_subBlocks.size() == _caseIdents.size());
        for (int idx = 0; idx < _subBlocks.size(); idx++){

            std::string finalized_case_id = _caseIdents[idx] == -1 ? "default"
                                                                 : std::to_string(_caseIdents[idx]);
            pre_ret += indent_val_inside + finalized_case_id + ":\n";
            pre_ret += indent_val_inside + " begin\n";
            pre_ret += _subBlocks[idx]->to_string(ident + 2* Verilog_IDENT) + "\n";
            pre_ret += indent_val_inside + " end\n";
        }

        pre_ret += indent_val + "endcase\n";
        return pre_ret;
    }



}
