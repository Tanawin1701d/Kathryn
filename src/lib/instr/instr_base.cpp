//
// Created by tanawin on 9/7/2024.
//

#include "instr_base.h"
#include "model/flow_block/cond/zelif.h"
#include "model/flow_block/cond/zif.h"
#include "util/numberic/pmath.h"
#include "util/term_color/term_color.h"
#include "util/str/str_util.h"


namespace kathryn{
    ////////////////////////// OPR HW

    OPR_HW::OPR_HW(int arch_size, int idx_size, int reg_no, bool is_src):
    _isSrc(is_src),
    data (m_opr_reg("reg_data"  + std::to_string(reg_no) + (is_src ? "Src": "Des"), arch_size)),
    idx  (m_opr_reg("reg_idx"   + std::to_string(reg_no) + (is_src ? "Src": "Des"), idx_size)),
    valid(m_opr_reg("reg_valid" + std::to_string(reg_no) + (is_src ? "Src": "Des"), 1)){
        assert( (1 << idx_size) >= arch_size);
        assert(reg_no >= 0);
    }

    void OPR_HW::reset(){
        valid <<= 0;
        idx   <<= 0;
    }

    void OPR_HW::set_only_index(Operable* index){
        assert(index != nullptr);
        assert(index->get_operable_slice().get_size() == idx.get_operable_slice().get_size());
        valid <<= 0;
        idx   <<= *index;
    }

    void OPR_HW::set_imm(Operable* opr){
        assert(opr != nullptr);
        assert(opr->get_operable_slice().get_size() == data.get_operable_slice().get_size());
        valid <<= 1;
        idx   <<= 0;
        data  <<= *opr;
    }



    ////////////////////////// OP HW
    OP_HW::OP_HW(int mop_idx,
                 const std::string& mop_name,
                 std::vector<std::string>& uop_names):
    _mopIdx(mop_idx),
    _mopName(mop_name),
    _set(m_opr_reg("mop_" + mop_name, 1)){
        assert(mop_idx >= 0);
        for(const std::string& uop_name: uop_names){
            int uop_idx = _uopSets.size();
            _uopSets.push_back(&m_opr_reg("uop_" + uop_name, 1));
            assert(uop_map_idx.find(uop_name)==uop_map_idx.end());
            uop_map_idx.insert({uop_name, uop_idx});
        }
    }

    void OP_HW::set(){ _set <<= 1; std::cout << " SET" << _mopName << std::endl;}

    void OP_HW::reset(){_set <<= 0; std::cout << " XXX" << _mopName << std::endl;}

    void OP_HW::set_uop(const std::string& uop_name, Operable* condition){
        assert(condition != nullptr);
        assert(condition->get_operable_slice().get_size() == 1);
        assert(uop_map_idx.find(uop_name) != uop_map_idx.end());
        int idx = uop_map_idx[uop_name];
        (*_uopSets.at(idx)) <<= (*condition);
    }

    Reg& OP_HW::is_set(){ return _set;}

    Reg& OP_HW::is_uop_set(const std::string& uop_name){
        assert(uop_map_idx.find(uop_name) != uop_map_idx.end());
        int uop_idx = uop_map_idx[uop_name];
        assert(uop_idx < _uopSets.size());
        assert(_uopSets[uop_idx] != nullptr);
        return *_uopSets[uop_idx];
    }

    bool OP_HW::is_there_uop(const std::string& uop_name){
        return uop_map_idx.find(uop_name) != uop_map_idx.end();
    }

    ////////////////////
    /////// InstrRepo
    ////////////////////


    bool MOP_META::check_valid() const{

        std::set<std::string> x;

        for (const auto& uop_name: uop_names){
            if(x.find(uop_name) != x.end()){
                return false;
            }
            x.insert(uop_name);
        }
        return true;

    }


    std::pair<
            std::vector<MasterRule*>,
            std::vector<MasterRule*>
    >
    InstrRepo::seperate_mop_byopcode(std::vector<MasterRule*>& msrs, int bit_idx){
        std::vector<MasterRule*> mop_with_zero;
        std::vector<MasterRule*> mop_with_one;

        for (MasterRule* msr: msrs){  //// msr master rule
            assert(msr != nullptr);
            char dec_val = msr->get_flatten_op()[bit_idx];
            switch(dec_val){
            case '0' : {mop_with_zero.push_back(msr); break;}
            case '1' : {mop_with_one .push_back(msr); break;}
            default : {assert(false);}
            }
        }
        return {mop_with_zero, mop_with_one};
    }

    void InstrRepo::gen_dec_internal(std::vector<MasterRule*>& msrs, int bit_idx){
        if (msrs.empty()){return;}

        if (bit_idx == -1){
            assert(msrs.size() == 1); ///// there must be only one
            std::cout << "do asm for " << msrs[0]->_mopName << std::endl;
            msrs[0]->do_asm();
            std::cout << "---------------------------" << std::endl;
            return;
        }

        auto [zeros, ones] =
            seperate_mop_byopcode(msrs, bit_idx);

        bool zero_empty = zeros.empty();
        bool one_empty  = ones .empty();

        if ((!zero_empty) && (!one_empty)){
            ////// seperate into two section if get condition
            ////// and not condition
            int arch_bit = INSTR_WIDTH - 1 - bit_idx;
            zif(
                *( (*_instr).do_slice({arch_bit, arch_bit+1}) )
            ){
                gen_dec_internal(ones, bit_idx-1);
            }zelse{
                gen_dec_internal(zeros, bit_idx-1);
            }
        }else{
            /////// the bit cannot be classified
            /// next find the other bit
            gen_dec_internal(ones , bit_idx-1);
            gen_dec_internal(zeros, bit_idx-1);
        }

    }


    InstrRepo::InstrRepo(int instr_width, int amt_src_opr,
                         int amt_des_opr , int opr_width,
                         Operable* instr):
        _instr     (instr     ),
        INSTR_WIDTH(instr_width),
        OPR_WIDTH  (opr_width  ),
        _amtMopType(0),
        _amtSrcOpr (amt_src_opr),
        _amtDesOpr (amt_des_opr){
        assert(instr != nullptr);
        assert(INSTR_WIDTH  > 0);
        assert(OPR_WIDTH    > 0);
        assert(amt_src_opr   >= 0);
        assert(amt_des_opr   >= 0);
    }

    InstrRepo::~InstrRepo(){
        for(OPR_HW* opr_hw: src_oprs){
            delete opr_hw;
        }
        for(OPR_HW* opr_hw: des_oprs){
            delete opr_hw;
        }
        for(OP_HW*   op_hw: opcodes){
            delete op_hw;
        }
    }

    void InstrRepo::add_mop(const MOP_META& mop_meta){
        assert(mop_meta.check_valid());
        assert(opcode_map.find(mop_meta.mop_name) == opcode_map.end());
        opcode_map.insert({mop_meta.mop_name, mop_metas.size()});
        mop_metas.push_back(mop_meta);
        _amtMopType++;
    }

    MasterRule& InstrRepo::add_dec_rule(const std::string& work_on_mop_name,
                               const std::string& rule){
        master_rules.emplace_back(this, work_on_mop_name, rule);
        return *master_rules.rbegin();
    }

    void InstrRepo::add_fix_prefix(const std::string& prefix_rule){
        _prefixCheck = TOKEN_GRP(prefix_rule, true);
    }

    void InstrRepo::declare_hw(){
        //////// declare_operand
        for (int i = 0; i < _amtSrcOpr; i++){
            src_oprs.push_back(new OPR_HW(
                OPR_WIDTH,
                log2Ceil(OPR_WIDTH),
                i, true));
        }
        for (int i = 0; i < _amtDesOpr; i++){
            des_oprs.push_back(new OPR_HW(
                OPR_WIDTH,
                log2Ceil(OPR_WIDTH),
                i, false));
        }

        for (int i = 0; i < mop_metas.size(); i++){
            opcodes.push_back(
                new OP_HW(i,mop_metas[i].mop_name,
                    mop_metas[i].uop_names));
        }
    }


    void InstrRepo::gen_decode_logic(){

        std::vector<MasterRule*> master_rule_pool;
        for (MasterRule& msr: master_rules){
            master_rule_pool.push_back(&msr);
        }
        if (_prefixCheck._tokens.empty()){
            gen_dec_internal(master_rule_pool, INSTR_WIDTH-1);
        }else{
            assert(_prefixCheck._tokens.size() == 1);
            token& prefix = _prefixCheck._tokens[0];
            ull check_val = stoi(prefix.splited_value[0], nullptr, 2);
            zif ( (*_instr->do_slice(prefix.sl)) == check_val){
                gen_dec_internal(master_rule_pool, INSTR_WIDTH-1);
            }zelse{

                for (OP_HW* op_hw: opcodes){
                    assert(op_hw != nullptr);
                    op_hw->reset();
                }


            }
        }

    }

    OPR_HW& InstrRepo::get_src_reg(int idx){
        assert(idx < _amtSrcOpr);
        return *src_oprs[idx];
    }
    OPR_HW& InstrRepo::get_des_reg(int idx){
        assert(idx < _amtDesOpr);
        return *des_oprs[idx];
    }
    OP_HW&  InstrRepo::get_op    (int mop_idx){
        assert(mop_idx < _amtMopType);
        return *opcodes[mop_idx];
    }
    OP_HW&  InstrRepo::get_op    (const std::string& mop_name){
        assert(opcode_map.find(mop_name) != opcode_map.end());
        return get_op(opcode_map[mop_name]);
    }


    InstrRepoDebugMsg InstrRepo::get_get_dbg_msg(){

        InstrRepoDebugMsg msg;

        std::set<std::string> running_mop;

        for(OP_HW* op_hw: opcodes){
            assert(op_hw != nullptr);
            if ((ull)op_hw->is_set()){
                running_mop.insert(op_hw->_mopName);
            }
        }

        if (running_mop.size() > 1){
            msg.error_cause += "multiple Op exist: ";
            for (auto mop_name: running_mop){
                msg.error_cause += mop_name + " ";
            }
            return msg;
        }else if (running_mop.size() == 0){
            msg.mop_name = "no Mop decoded";
            return msg;
        }
        msg.mop_name = *running_mop.begin();

        std::vector<std::string> running_uop;

        OP_HW* op_hw = &get_op(*running_mop.begin());
        for (const auto& [uop_name, idx]: op_hw->uop_map_idx){
            if ((ull)*op_hw->_uopSets[idx]){
                running_uop.push_back(uop_name);
                msg.uop_name += " " + uop_name;
                /////std::cout << "uop_name " << (ull)op_hw->_uopSets[idx] << std::endl;
            }
        }
        return msg;
    }

    bool InstrRepo::is_there_op_dec(const std::string& op_name){
        return opcode_map.find(op_name) != opcode_map.end();
    }

    bool InstrRepo::is_there_uop_dec(std::string op_name, const std::string& uop_name){
        if (!is_there_op_dec(op_name)){ return false;}


        for (const std::string& uop_in_grp: mop_metas[opcode_map[op_name]].uop_names){
            if (uop_name == uop_in_grp){ return true;}
        }
        return false;

    }



}
