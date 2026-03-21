//
// Created by tanawin on 19/7/2024.
//

#include "cpp_writer.h"

#include "utility"

#include "util/str/str_util.h"

namespace kathryn{
    /**
     *  CbBaseCxx
     */
    CbIfCxx& CbBaseCxx::add_if(std::string condition){
        auto* if_block = new CbIfCxx(false,std::move(condition));
        append_sub_block(if_block);
        return *if_block;
    }

    CbSwitchCxx& CbBaseCxx::add_switch(std::string switch_ident){
        auto* switch_block = new CbSwitchCxx(std::move(switch_ident));
        append_sub_block(switch_block);
        return *switch_block;
    }

    CbBaseCxx& CbBaseCxx::add_sub_block(){
        auto* sub_block = new CbBaseCxx();
        append_sub_block(sub_block);
        return *sub_block;
    }


    std::string CbBaseCxx::to_string(int ident){

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
                pre_ret += _subBlocks[next_sb_check_idx]->to_string(ident + CXX_IDENT);
                next_sb_check_idx++;
                continue;
            }
            assert(false);
        }
        return pre_ret;
    }

    /**
     *
     * CbIfCxx
     *
     */

    CbIfCxx::CbIfCxx(bool is_sub_chain, std::string condtion):
    _markAsSubChain(is_sub_chain),
    _cond(std::move(condtion)){}

    CbIfCxx& CbIfCxx::add_elif(std::string condition){
        CbIfCxx* elif_block = new CbIfCxx(true,std::move(condition));
        _contBlock.push_back(elif_block);
        return *elif_block;
    }

    std::string CbIfCxx::to_string(int ident){
        std::string pre_ret;
        std::string indent_val = gen_con_string(' ', ident);

        if (!_markAsSubChain){
            pre_ret += indent_val + "if(" + _cond + ")";
        }else if (!_cond.empty()){
            pre_ret += "else if(" + _cond + ")";
        }else{
            pre_ret += "else";
        }

        pre_ret += "{\n";

        pre_ret += CbBaseCxx::to_string(ident + CXX_IDENT);

        pre_ret += indent_val + "}";

        for(CbIfCxx* cont_block: _contBlock){
            assert(cont_block != nullptr);
            pre_ret += cont_block->to_string(ident);
        }
        if (!_markAsSubChain){
            pre_ret += "\n";
        }
        return pre_ret;
    }

    /**
     *
     * CbSwitchFunc
     *
     */

    CbBaseCxx& CbSwitchCxx::add_case(int case_val){

        if (case_val == -1){
            assert(!is_default_occure);
            is_default_occure = true;
        }
        _caseIdents.push_back(case_val);
        return CbBaseCxx::add_sub_block();

    }

    std::string CbSwitchCxx::to_string(int ident){
        std::string pre_ret;
        std::string indent_val = gen_con_string(' ', ident);
        std::string indent_val_inside = gen_con_string(' ', ident + CXX_IDENT);

        pre_ret += indent_val + "switch(" + _switchIdent + "){\n";

        assert(_subBlocks.size() == _caseIdents.size());
        for (int idx = 0; idx < _subBlocks.size(); idx++){

            /////// add break to break
            _subBlocks[idx]->add_st("break", true);

            std::string finalized_case_str = _caseIdents[idx] == -1 ? "default"
                                                                 : "case " + std::to_string(_caseIdents[idx]);
            pre_ret += indent_val_inside +  finalized_case_str + ":\n";
            pre_ret += indent_val_inside + " {\n";
            pre_ret += _subBlocks[idx]->to_string(ident + 2* CXX_IDENT) + "\n";
            pre_ret += indent_val_inside + " }\n";
        }

        pre_ret += indent_val + "}";
        return pre_ret;
    }



}
