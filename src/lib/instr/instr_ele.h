//
// Created by tanawin on 9/7/2024.
//

#ifndef INSTRELE_H
#define INSTRELE_H

#include "map"
#include "vector"
#include "cassert"
#include "set"

#include "model/hw_component/abstract/slice.h"
#include "model/hw_component/register/register.h"
#include "model/hw_component/value/value.h"
#include "model/hw_component/expression/expression.h"


namespace kathryn{

    class InstrRepo;

    /////// for decoder
    constexpr  int TOKEN_DEC_TYPE_IDX = 1; //// at splited_value
    /////// for slave matching
    ///   <ua>
    constexpr  int TOKEN_ASM_UOP_IDENT= 0;
    ///   <s-25-28>
    constexpr  int TOKEN_ASM_UOP_DIRECT_START_IDX = 1;
    constexpr  int TOKEN_ASM_UOP_DIRECT_STOP_IDX  = 2;


    constexpr  char TOKEN_ASM_TYPE_REG_IDX      = 'r'; //// register idx
    constexpr  char TOKEN_ASM_TYPE_IMM_IDX      = 'i'; //// imm idx
    constexpr  char TOKEN_ASM_TYPE_UOP_IDX      = 'u'; //// imm idx
    constexpr  char TOKEN_ASM_TYPE_UOP_DIR_IDX  = 's'; //// imm idx


    Operable* join_opr(std::vector<Operable*> src_oprs, LOGIC_OP lop);

    struct token{
        bool  is_size_det = false;
        Slice sl{};
        std::string value;
        std::vector<std::string> splited_value; ///// after split - such as <5-rd0> -> {5, rd0}
        explicit token() = default;
        void split_token();
        void size_det(int start_bit);
        void add_raw_char(char ch){value.push_back(ch);}
    };

    struct TOKEN_GRP{
        std::vector<token> _tokens;
        TOKEN_GRP(std::string raw_value, bool req_size_det);
        TOKEN_GRP() = default;
        /////// it will determine size
    };

    ///// the component that used to be instruction and micro-op material
    struct AsmWorker{
        InstrRepo* _master = nullptr;
        AsmWorker(InstrRepo* master): _master(master){ assert(master != nullptr);}
        virtual ~AsmWorker() = default;
        virtual void do_asm() = 0;
    };

    struct RegIdxAsm: AsmWorker{
        ///// for src register <#{bitsize}-rs#{idx}>
        const int TOKEN_TYPE_IDX = 1;
        const int TOKEN_TYPE_START_IDX = 0;
        const int TOKEN_TYPE_END_IDX   = 2;
        const int TOKEN_REG_NUM_START_IDX = 2;
        const char TOKEN_TYPE_SRC[3] = "rs";
        const char TOKEN_TYPE_DES[3] = "rd";
        bool  is_read  = false; // else is Write
        int   _regCnt = -1;
        Slice _srcSlice = {};

        explicit RegIdxAsm(InstrRepo* master,const token& tk);
        void     do_asm() override;
    };

    struct ImmAsm: AsmWorker{
        /// for <#{bitsize}-i#{idx}-#{start_bit}-#{stop_bit}-#{zero_extend? z: e}>
        /// ----> [start_bit, stop_bit)
        const char ZEROEXTEND       = 'z';
        const int TOKEN_TYPE_IDX    = 1;
        const int TOKEN_TYPE_SIZE   = 1;
        const int TOKEN_FILLA_IDX   = 2;
        const int TOKEN_FILLB_IDX   = 3;
        const int TOKEN_FILLEXT_IDX = 4;
        struct IterImm{
            bool  need_dummy_src = false; /// src Slice will be neglected
            Slice src_slice;
            Slice des_slice;
            Operable* sliced = nullptr;
            bool operator < (const IterImm& rhs){
                return des_slice.start < rhs.des_slice.start;
            }
        };
        std::vector<IterImm> imm_slicer;
        int  _regCnt     = -1;
        bool _signedExtend = true;

        explicit ImmAsm(InstrRepo* master): AsmWorker(master){};
        void     add_imm_meta(const token& tk);
        void     do_asm() override;

    };

    struct MatchSlaveVal{
        TOKEN_GRP    _matchRule;
        TOKEN_GRP    _matcher;
        std::string  _effUopName;

        MatchSlaveVal(const std::string& match_rule,
                      const std::string& match_val,
                      std::string eff_uop_name);

        Operable*get_act_cond_ptr(std::map<std::string, token> slave_token_map, Operable* instr);

    };

    struct MatchSlaveValInput{
        std::string  match_val;
        std::string  eff_uop;
    };


    struct MasterRule{
        InstrRepo*         _master = nullptr;
        std::string        _mopName;
        TOKEN_GRP          _masterTokens;
        std::vector<token> _opcodeTokens;
        std::string        _flattenOp;
        /////// register
        std::vector<RegIdxAsm>  _srcRegAsms;
        std::vector<RegIdxAsm>  _desRegAsms;
        ImmAsm _immAsm;

        /////// slave side
        std::map<std::string, token> _slaveTokenMap; //// name of uop_identfier like u2 -> token
        std::vector<MatchSlaveVal>   _slaveRules;
        std::set<std::string>        _dummyUopSetter;

        MasterRule(InstrRepo* repo, std::string mop_name, std::string rule);
        void flatten_mop();
        [[nodiscard]]
        std::string get_flatten_op() const{return _flattenOp;}

        ////////// add decode
        MasterRule& ad(const std::string& slave_rule,
                                const std::vector<MatchSlaveValInput>& slave_match);

        MasterRule& adm(const std::vector<std::string>& uops_name);

        void do_asm();

        void do_uop_asm();

        void unset_unused_reg(const bool* eff, int size, bool is_src);

    };

    ////// todo



}

#endif //INSTRELE_H
