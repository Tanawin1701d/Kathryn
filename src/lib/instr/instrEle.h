//
// Created by tanawin on 9/7/2024.
//

#ifndef INSTRELE_H
#define INSTRELE_H

#include<map>
#include<vector>
#include<cassert>
#include <set>

#include "model/hwComponent/abstract/Slice.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/value/value.h"
#include "model/hwComponent/expression/expression.h"


namespace kathryn{

    class InstrRepo;

    /////// for decoder
    constexpr  int TOKEN_DEC_TYPE_IDX = 1; //// at splitedValue
    /////// for slave matching
    constexpr  int TOKEN_ASM_UOP_IDENT= 0;
    constexpr  char TOKEN_ASM_TYPE_REG_IDX  = 'r'; //// register idx
    constexpr  char TOKEN_ASM_TYPE_IMM_IDX  = 'i'; //// imm idx
    constexpr  char TOKEN_ASM_TYPE_UOP_IDX  = 'u'; //// imm idx

    Operable* joinOpr(std::vector<Operable*> srcOprs, LOGIC_OP lop);

    struct token{
        bool  isSizeDet = false;
        Slice sl{};
        std::string value;
        std::vector<std::string> splitedValue;
        explicit token() = default;
        void splitToken();
        void sizeDet(int startBit);
        void addRawChar(char ch){value.push_back(ch);}
    };

    struct TOKEN_GRP{
        std::vector<token> _tokens;
        TOKEN_GRP(std::string rawValue, bool reqSizeDet);
        /////// it will determine size
    };

    ///// the component that used to be instruction and micro-op material
    struct AsmWorker{
        InstrRepo* _master = nullptr;
        AsmWorker(InstrRepo* master): _master(master){ assert(master != nullptr);}
        virtual ~AsmWorker() = default;
        virtual void doAsm() = 0;
    };

    struct RegIdxAsm: AsmWorker{
        ///// for src register <#{bitsize}-rs#{idx}>
        const int TOKEN_TYPE_IDX = 1;
        const int TOKEN_TYPE_START_IDX = 0;
        const int TOKEN_TYPE_END_IDX   = 2;
        const int TOKEN_REG_NUM_START_IDX = 2;
        const char TOKEN_TYPE_SRC[3] = "rs";
        const char TOKEN_TYPE_DES[3] = "rd";
        bool  isRead  = false; // else is Write
        int   _regCnt = -1;
        Slice _srcSlice = {};

        explicit RegIdxAsm(InstrRepo* master,const token& tk);
        void     doAsm() override;
    };

    struct ImmAsm: AsmWorker{
        /// for <#{bitsize}-i#{idx}-#{startBit}-#{stopBit}-#{zeroExtend? z: e}>
        /// ----> [startBit, stopBit)
        const char ZEROEXTEND       = 'z';
        const int TOKEN_TYPE_IDX    = 1;
        const int TOKEN_TYPE_SIZE   = 1;
        const int TOKEN_FILLA_IDX   = 2;
        const int TOKEN_FILLB_IDX   = 3;
        const int TOKEN_FILLEXT_IDX = 4;
        struct IterImm{
            bool  needDummySrc = false; /// src Slice will be neglected
            Slice srcSlice;
            Slice desSlice;
            Operable* sliced = nullptr;
            bool operator < (const IterImm& rhs){
                return desSlice.start < rhs.desSlice.start;
            }
        };
        std::vector<IterImm> immSlicer;
        int  _regCnt     = -1;
        bool _signedExtend = true;

        explicit ImmAsm(InstrRepo* master): AsmWorker(master){};
        void     addImmMeta(const token& tk);
        void     doAsm() override;

    };

    struct MatchSlaveVal{
        TOKEN_GRP    _matchRule;
        TOKEN_GRP    _matcher;
        std::string  _effUopName;

        MatchSlaveVal(const std::string& matchRule,
                      const std::string& matchVal,
                      std::string effUopName);

        Operable* getActCond(std::map<std::string, token> slaveTokenMap, Operable* instr);

    };

    struct MatchSlaveValInput{
        std::string  matchVal;
        std::string  effUop;
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
        std::map<std::string, token> _slaveTokenMap; //// name of uopIdentfier like u2 -> token
        std::vector<MatchSlaveVal>   _slaveRules;
        std::set<std::string>        _dummyUopSetter;

        MasterRule(InstrRepo* repo, std::string mopName, std::string rule);
        void flattenMop();
        [[nodiscard]]
        std::string getFlattenOp() const{return _flattenOp;}

        ////////// add decode
        MasterRule& ad(const std::string& slaveRule,
                                const std::vector<MatchSlaveValInput>& slaveMatch);

        MasterRule& adm(const std::vector<std::string>& uopsName);

        void doAsm();

        void doUopAsm();

        void unsetUnusedReg(const bool* eff, int size, bool isSrc);

    };

    ////// todo



}

#endif //INSTRELE_H
