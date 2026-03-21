//
// Created by tanawin on 9/7/2024.
//
#include "instr_ele.h"
#include "instr_base.h"
#include "utility"
#include "model/hw_component/expression/nest.h"

#include "util/str/str_util.h"

namespace kathryn{

    Operable* join_opr(std::vector<Operable*> src_oprs, LOGIC_OP lop){
        assert(!src_oprs.empty());
        Operable* result = src_oprs[0];

        for (int i = 1; i < src_oprs.size(); i++){
            switch(lop){
            case BITWISE_AND:{
                result = &((*result) & (*src_oprs[i]));
                break;
            }
            case BITWISE_OR:{
                result = &((*result) | (*src_oprs[i]));
                break;
            }
            default: {assert(false);}
            }
        }
        return result;
    }

    /***
    *
    * TOKEN
    *
    */

    void token::split_token(){
        splited_value = split_str(value, '-');
    }

    void token::size_det(int start_bit){
        /////// sl.start is filled you must compute sl.stop
        try{
            assert(start_bit >= 0);
            sl.start = start_bit;
            if(splited_value.size() == 1){
                sl.stop = sl.start + ((int)value.size()); //// <0000011>
            }else{
                sl.stop = sl.start  + std::stoi(splited_value[0]); ///// <5-rd0> use 5
            }
            assert(sl.check_valid_slice());
            is_size_det = true;
        }catch (...){
            assert(false); /// can't determine size
        }
    }


    //////////////////////////
    /// TOKEN_GRP
    //////////////////////////

    TOKEN_GRP::TOKEN_GRP(std::string raw_value, bool req_size_det){
        std::stack<token> token_st;
        /////// iterate all in string
        for (int idx = 0; idx < raw_value.size(); idx++){
            char bit_char = raw_value[idx];
            switch (bit_char){
            case '<':{token_st.emplace(); break;}
            case '>':{
                token top_token = token_st.top();
                top_token.split_token();
                _tokens.push_back(top_token);
                token_st.pop();
                break;
            }
            default:{
                assert(!token_st.empty());
                token_st.top().add_raw_char(bit_char); break;
            }
            }
        }

        if (req_size_det){
            int next_start_idx = 0;
            for (auto tk = _tokens.rbegin(); tk != _tokens.rend(); tk++){
                tk->size_det(next_start_idx);
                next_start_idx += tk->sl.get_size();
            }
        }
        assert(token_st.empty());
    }

    /**
     *
     * reg idx asm
     *
     ***/

    RegIdxAsm::RegIdxAsm(InstrRepo* master, const token& tk): AsmWorker(master){
        std::string asm_type = tk.splited_value[TOKEN_TYPE_IDX].
                               substr(TOKEN_TYPE_START_IDX,
                                      TOKEN_TYPE_END_IDX);
        if (asm_type == TOKEN_TYPE_SRC){
            is_read = true;
        }
        if (asm_type == TOKEN_TYPE_DES){
            is_read = false;
        }
        _regCnt = std::stoi(tk.splited_value[TOKEN_TYPE_IDX].
                            substr(TOKEN_REG_NUM_START_IDX));
        _srcSlice = tk.sl;

        assert(_regCnt >= 0);
        if ( is_read){ assert(_regCnt < _master->get_amt_src_reg());}
        if (!is_read){ assert(_regCnt < _master->get_amt_des_reg());}
        assert(_srcSlice.stop <= _master->get_opr_size());
        assert(_srcSlice.check_valid_slice());
    }

    void RegIdxAsm::do_asm(){
        assert(_master != nullptr);
        OPR_HW& opr1 = is_read ? _master->get_src_reg(_regCnt)
                              : _master->get_des_reg(_regCnt);
        auto sliced_reg_file_idx = _master->get_instr_opr_ptr()->do_slice(_srcSlice);
        opr1.set_only_index(sliced_reg_file_idx);
    }


    /**
     *
     * imm idx asm
     *
     ***/

    void ImmAsm::add_imm_meta(const token& tk){
        assert(tk.splited_value.size() >= (TOKEN_FILLB_IDX+1));
        assert(tk.sl.check_valid_slice());

        // fill to [a, b)
        _regCnt   = std::stoi(tk.splited_value[TOKEN_TYPE_IDX].substr(TOKEN_TYPE_SIZE)); //// start number to end
        int fill_a = std::stoi(tk.splited_value[TOKEN_FILLA_IDX]);
        int fill_b = std::stoi(tk.splited_value[TOKEN_FILLB_IDX]);
        Slice fill_sl = {fill_a, fill_b};
        assert((fill_b-fill_a) == std::stoi(tk.splited_value[0]));
        assert(fill_sl.check_valid_slice());
        imm_slicer.push_back({
            false,
            tk.sl,
            fill_sl,
            _master->get_instr_opr_ptr()->do_slice(tk.sl)
        });

        if ( (tk.splited_value.size() > TOKEN_FILLEXT_IDX) &&
             (tk.splited_value[TOKEN_FILLEXT_IDX][0] == ZEROEXTEND)){
            _signedExtend = false;
        }
    }

    void ImmAsm::do_asm(){

        if (imm_slicer.empty()){
            return;
        }

        ////// start sort first
        std::sort(imm_slicer.begin(), imm_slicer.end());
        assert(!imm_slicer.empty());
        std::vector<IterImm> arranged_opr;
        ///// arrange all imm to proper slot and make dummy if there is no
        /// imm to fill that slot fill it with dummy initialize the dummy
        int cur_start_idx = 0;
        for(int idx = 0; idx < imm_slicer.size(); idx++){
            IterImm& iter_imm = imm_slicer[idx];
            if (cur_start_idx != iter_imm.des_slice.start){
                arranged_opr.push_back({true,{-1,-1}, {cur_start_idx, iter_imm.des_slice.start}, nullptr});
            }
            arranged_opr.push_back(iter_imm);
            cur_start_idx = iter_imm.des_slice.stop;
        }
        assert(!arranged_opr.empty());
        assert(!arranged_opr.rbegin()->need_dummy_src);
        if (cur_start_idx != _master->get_opr_size()){
            arranged_opr.push_back({true, {-1,-1}, {cur_start_idx, _master->get_opr_size()}, nullptr});
        }
        cur_start_idx = _master->get_opr_size();

        //// initialize the dummy

        for(int idx = 0; idx < (((int)arranged_opr.size())-1); idx++){
            if (arranged_opr[idx].need_dummy_src){
                arranged_opr[idx].sliced =
                    &make_opr_val("imm_fill", arranged_opr[idx].des_slice.get_size(), 0);
            }
        }

        //// do extend bit if there needed
        if(arranged_opr.rbegin()->need_dummy_src){
            assert(arranged_opr.size() >= 2);
            IterImm& last_iter    = *arranged_opr.rbegin();
            IterImm& src_to_extend = *(arranged_opr.rbegin()+1);
            if (_signedExtend){
                Operable* src_opr = src_to_extend.sliced;
                int src_size = src_opr->get_operable_slice().get_size();
                Operable* onlylast_bit = src_opr->do_slice({src_size-1, src_size});
                last_iter.sliced  = &onlylast_bit->ext_b(last_iter.des_slice.get_size());
            }else{
                last_iter.sliced  = &make_opr_val("last_imm_f_ill", last_iter.des_slice.get_size(), 0);
            }
        }


        ///// do nest
        std::vector<Operable*> metas;
        for(IterImm imm: arranged_opr){
            metas.push_back(imm.sliced);
        }
        nest& nested = g_man_internal_read_only(metas);
        //// put it to operand
        OPR_HW& opr = _master->get_src_reg(_regCnt);
        opr.set_imm(&nested);
    }



    /////////////////////////
    /// MatchSlaveVal
    /////////////////////////

    MatchSlaveVal::MatchSlaveVal(const std::string& match_rule,
                                 const std::string& match_val,
                                 std::string uop_name):
    _matchRule(match_rule, false),
    _matcher(match_val, false),
    _effUopName(std::move(uop_name)){}

    Operable* MatchSlaveVal::get_act_cond_ptr(
    std::map<std::string, token> slave_token_map,
    Operable* instr
    ){
        /////// traverse to our rule
        assert(_matchRule._tokens.size() == _matcher._tokens.size());
        std::vector<Operable*> activate_conditions;
        for (int match_idx = 0; match_idx < _matchRule._tokens.size(); match_idx++){
            token match_rule_token = _matchRule._tokens[match_idx];

            token match_value_token = _matcher._tokens[match_idx];

            Slice target_sl = {-1,-1};
            if (match_rule_token.splited_value[TOKEN_ASM_UOP_IDENT][0] == TOKEN_ASM_TYPE_UOP_IDX){
                token instr_slice_token = slave_token_map[match_rule_token.splited_value[TOKEN_ASM_UOP_IDENT]];
                target_sl = instr_slice_token.sl;
            }else if (match_rule_token.splited_value[TOKEN_ASM_UOP_IDENT][0] == TOKEN_ASM_TYPE_UOP_DIR_IDX){
                target_sl = {std::stoi(match_rule_token.splited_value[TOKEN_ASM_UOP_DIRECT_START_IDX]),
                                 std::stoi(match_rule_token.splited_value[TOKEN_ASM_UOP_DIRECT_STOP_IDX])};
            }
            assert(target_sl.check_valid_slice());
            assert(target_sl.stop <= instr->get_operable_slice().get_size());


            Operable* sliced_instr = instr->do_slice(target_sl);

            assert(match_rule_token.sl == Slice());
            assert(match_value_token.sl == Slice());

            std::string raw_match_value = match_value_token.splited_value[TOKEN_ASM_UOP_IDENT];
            ull         match_value   = std::stoi(raw_match_value, nullptr, 2);

            activate_conditions.push_back( &((*sliced_instr) == match_value));
        }

        assert(!activate_conditions.empty());
        return join_opr(activate_conditions, BITWISE_AND);
    }

    ////////////////////////////
    /// MasterRule
    ////////////////////////////
    ///

    MasterRule::MasterRule
    (InstrRepo* repo, std::string mop_name,std::string rule):
    _master(repo),
    _mopName(std::move(mop_name)),
    _masterTokens(std::move(rule), true),
    _immAsm(_master){

        ///////////////// inteprt The token
        for (const token& tk: _masterTokens._tokens){
            std::vector<std::string> str_vec_tk = tk.splited_value;
            assert(!str_vec_tk.empty());
            /////case OP
            if (str_vec_tk.size() == 1){
                /////// it is op
                _opcodeTokens.push_back(tk); ////// the main op decoder <0100011>
                continue;
            }
            //// case REG
            if(str_vec_tk[TOKEN_DEC_TYPE_IDX][0] == TOKEN_ASM_TYPE_REG_IDX){
                RegIdxAsm worker(_master,tk);
                if (worker.is_read){_srcRegAsms.push_back(worker);}
                else              {_desRegAsms.push_back(worker);}
                continue;
            }
            //// case IMM
            if(str_vec_tk[TOKEN_DEC_TYPE_IDX][0] == TOKEN_ASM_TYPE_IMM_IDX){
                _immAsm.add_imm_meta(tk);
                continue;
            }
            //// case UOP
            if(str_vec_tk[TOKEN_DEC_TYPE_IDX][0] == TOKEN_ASM_TYPE_UOP_IDX){
                ///// u1
                std::string uop_ident = str_vec_tk[TOKEN_DEC_TYPE_IDX];
                //////////                    v------------ ujanway   -> token
                _slaveTokenMap.insert({uop_ident, tk});
            }
        }
        ////// flatten the mop
        flatten_mop();

    }

    void MasterRule::flatten_mop(){
        _flattenOp = gen_con_string('0', _master->get_instr_size());
        for(token& tk: _opcodeTokens){
            int actual_str_start = _master->get_instr_size() - tk.sl.stop;
            int actual_str_stop  = _master->get_instr_size() - tk.sl.start;
            assert((actual_str_stop-actual_str_start) == tk.sl.get_size());
            _flattenOp.replace(
                actual_str_start,
                actual_str_stop-actual_str_start,
                tk.splited_value[0]
            );
        }
    }

    MasterRule& MasterRule::ad(const std::string& slave_rule,
                                        const std::vector<MatchSlaveValInput>&
                                        slave_match){
        for(const auto& match : slave_match){
            ////// check uop first is match uop

            assert(_master->is_there_uop_dec(_mopName, match.eff_uop));
            _slaveRules.emplace_back(slave_rule, match.match_val, match.eff_uop);
        }
        return *this;
    }

    MasterRule& MasterRule::adm(const std::vector<std::string>& uops_name){
        for (const auto& uop_name: uops_name){
            _dummyUopSetter.insert(uop_name);
        }
        return *this;
    }


    void MasterRule::do_asm(){

        bool eff_src[_master->get_amt_src_reg()] = {};
        bool eff_des[_master->get_amt_des_reg()] = {};
        /** declare assignment and check the effective hardware*/
        /** for register*/
        for (RegIdxAsm& src_reg_asm: _srcRegAsms){
            src_reg_asm.do_asm();
            eff_src[src_reg_asm._regCnt] = true; //// mark it is used
        }
        for (RegIdxAsm& des_reg_asm: _desRegAsms){
            des_reg_asm.do_asm();
            eff_des[des_reg_asm._regCnt] = true;
        }
        _immAsm.do_asm();
        if (_immAsm._regCnt != -1){
            eff_src[_immAsm._regCnt] = true;
        }

        /** disable unused hw*/
        unset_unused_reg(eff_src, _master->get_amt_src_reg(), true);
        unset_unused_reg(eff_des, _master->get_amt_des_reg(), false);

        /* set uop hardware*/
        do_uop_asm();


    }

    void MasterRule::do_uop_asm(){

        /////// get mop hardware
        OP_HW& op_hw = _master->get_op(_mopName);
        int mop_idx = op_hw._mopIdx;

        for(const auto&[uop_name,uop_idx]: op_hw.uop_map_idx){
            //////////////// check dummy first
            if (_dummyUopSetter.find(uop_name) != _dummyUopSetter.end()){
                op_hw.set_uop(uop_name, &make_opr_val("set_dummy_start",1,1));
                continue;
            }
            //////////////// search all uop
            std::vector<Operable*> activate_cons; ///// the activation condition for
            for(auto slave_rule:  _slaveRules){
                if (slave_rule._effUopName == uop_name){
                    activate_cons.push_back(
                        slave_rule.get_act_cond_ptr(_slaveTokenMap, _master->get_instr_opr_ptr()));
                }
            }
            if (activate_cons.empty()){
                op_hw.set_uop(uop_name, &make_opr_val("set_dummy",1,0));
            }else{
                op_hw.set_uop(uop_name, join_opr(activate_cons, BITWISE_OR));
            }
        }

        ///////// unset all backward mopname

        for (int i =0; i < _master->get_amt_mop(); i++){
            if (i != mop_idx){
                _master->get_op(i).reset();
            }else{
                _master->get_op(i).set();
            }
        }


    }

    void MasterRule::unset_unused_reg(const bool* eff, int size, bool is_src){
        for (int reg_idx = 0; reg_idx < size; reg_idx++){
            OPR_HW& reg_hw = is_src ? _master->get_src_reg(reg_idx) : _master->get_des_reg(reg_idx);
            if (!eff[reg_idx]){
                reg_hw.reset();
            }
        }
    }
}