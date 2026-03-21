//
// Created by tanawin on 9/7/2024.
//

#ifndef INSTRREP_H
#define INSTRREP_H
#include "vector"
#include "map"
#include "set"
#include "utility"


#include "model/hw_component/register/register.h"
#include "model/hw_component/abstract/slice.h"
#include "instr_ele.h"


namespace kathryn{
    // "<7-op3><5-r2><1-IMMZ><5-r1>0011100011"

    /***  the hardware stucture*/

    struct InstrRepoDebugMsg{
        std::string mop_name;
        std::string uop_name;
        std::string error_cause;
    };

    struct OPR_HW{
        bool _isSrc = true;
        Reg& valid;
        Reg& data;
        Reg& idx;

        explicit OPR_HW(int arch_size, int idx_size, int reg_no, bool is_src);
        void reset();
        void set_only_index(Operable* index); ///// the value must get manual from regfile
        void set_imm(Operable* value); ///// the value will be added and valid

        OPR_HW& operator <<=(const OPR_HW& rhs){
            valid <<= rhs.valid;
            data  <<= rhs.data;
            idx   <<= rhs.idx;
            return *this;
        }
    };
    struct OP_HW{
        int _mopIdx = -1;
        std::string _mopName;
        Reg& _set;
        std::vector<Reg*> _uopSets;
        std::map<std::string, int> uop_map_idx;
        explicit OP_HW(int mop_idx,
                       const std::string& mop_name,
                       std::vector<std::string>& uops);
        ////// operation on reg
        void set();
        void set_uop(const std::string& idx, Operable* condition);
        void reset();
        ////// get value
        Reg& is_set();
        Reg& is_uop_set(const std::string& uop_name);

        bool is_there_uop(const std::string& uop_name);
    };
    struct MOP_META{
        std::string mop_name;
        std::vector<std::string> uop_names;

        [[nodiscard]] bool check_valid() const;
    };

    class InstrRepo{
    protected:
        Operable* _instr = nullptr;
        ///// we don't allow to add rule any more
        ///// meta data
        const int INSTR_WIDTH    = -1;
        const int OPR_WIDTH      = -1;
              int _amtMopType    = -1;
        const int _amtSrcOpr     = -1;
        const int _amtDesOpr     = -1;
        TOKEN_GRP _prefixCheck;
        std::vector<MasterRule> master_rules;
        std::vector<MOP_META>   mop_metas;
        ////// hardware component
        std::vector<OPR_HW*> src_oprs;
        std::vector<OPR_HW*> des_oprs;
        std::vector<OP_HW* > opcodes; //// index is type idx
        std::map<std::string, int> opcode_map;
        ////// index in uop is uop_idx as well
        std::pair<
            std::vector<MasterRule*>,
            std::vector<MasterRule*>
        >    seperate_mop_byopcode(std::vector<MasterRule*>& msrs, int bit_idx); ///// master rules
        void gen_dec_internal     (std::vector<MasterRule*>& msrs, int bit_idx);

    public:

        explicit      InstrRepo(int instr_width, int amt_src_opr,
                                int amt_des_opr , int opr_width,
                                Operable* instr);
        virtual       ~InstrRepo();
        void          add_mop(const MOP_META& mop_meta);
        //////// add rule
        MasterRule&   add_dec_rule(const std::string& work_on_mop_name,
                                 const std::string& rule);
        void          add_fix_prefix(const std::string& pre_fix_rule);
        //////// hardware declaration
        void          declare_hw(); ////// define when add_mop is added that ok;
        //////// gen the logic
        virtual  void gen_decode_logic(); ////// gen logic

        //// src reg
        OPR_HW& get_src_reg   (int idx);
        int     get_amt_src_reg() const{return _amtSrcOpr;}
        OPR_HW& get_des_reg   (int idx);
        int     get_amt_des_reg() const{return _amtDesOpr;}
        OP_HW&  get_op       (int type_id);
        OP_HW&  get_op       (const std::string& mop_idx);
        int     get_amt_mop   ()     const{return (int)opcodes.size();}


        Operable* get_instr_opr_ptr() const{return _instr;}
        int     get_instr_size () const{return INSTR_WIDTH;}
        int     get_opr_size()    const{return OPR_WIDTH;}

        ////////// for debug msg
        ///
        InstrRepoDebugMsg get_get_dbg_msg();
        bool is_there_op_dec(const std::string& op_name);
        bool is_there_uop_dec(std::string op_name, const std::string& uop_name);

    };

}

#endif //INSTRREP_H
