//
// Created by tanawin on 9/7/2024.
//

#ifndef INSTRELE_H
#define INSTRELE_H

#include<vector>
#include<cassert>
#include "model/hwComponent/abstract/Slice.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/value/value.h"
#include "model/hwComponent/expression/expression.h"


namespace kathryn{

    class InstrRepo;

    constexpr  int TOKEN_ASM_TYPE_IDX = 1; //// at splitedValue
    constexpr  char TOKEN_ASM_TYPE_REG_IDX  = 'r'; //// register idx
    constexpr  char TOKEN_ASM_TYPE_IMM_IDX  = 'i'; //// imm idx
    constexpr  char TOKEN_ASM_TYPE_UOP_IDX  = 'u'; //// imm idx


    struct token{
        Slice sl{};
        std::string value;
        std::vector<std::string> splitedValue;
        explicit token() = default;
        void finalToken(int startBit);
        void addRawChar(char ch){value.push_back(ch);}
    };

    ///// the component that used to be instruction and micro-op material
    struct AsmWorker{
        InstrRepo* _master = nullptr;
        virtual ~AsmWorker() = default;
        virtual void doAsm() = 0;
        void setMaster(InstrRepo* master){
            assert(master != nullptr);
            _master = master;
        }
    };

    struct RegIdxAsm: AsmWorker{
        ///// for src register <#{bitsize}-rs#{idx}>
        constexpr int TOKEN_TYPE_IDX = 1;
        constexpr int TOKEN_TYPE_START_IDX = 0;
        constexpr int TOKEN_TYPE_END_IDX   = 2;
        constexpr int TOKEN_REG_NUM_START_IDX = 2;
        constexpr char TOKEN_TYPE_SRC[3] = "rs";
        constexpr char TOKEN_TYPE_DES[3] = "rd";
        bool  isRead  = false; // else is Write
        int   _regCnt = -1;
        Slice _srcSlice = {};

        explicit RegIdxAsm(const token& tk);
        void     doAsm() override;
    };

    struct ImmAsm: AsmWorker{
        /// for <#{bitsize}-i#{idx}-#{startBit}-#{stopBit}-#{zeroExtend? z: e}>
        /// ----> [startBit, stopBit)
        constexpr char ZEROEXTEND       = 'z';
        constexpr int TOKEN_TYPE_IDX    = 1;
        constexpr int TOKEN_TYPE_SIZE   = 1;
        constexpr int TOKEN_FILLA_IDX   = 2;
        constexpr int TOKEN_FILLB_IDX   = 3;
        constexpr int TOKEN_FILLEXT_IDX = 4;
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

        explicit ImmAsm() = default;
        void     addImmMeta(const token& tk);
        void     doAsm() override;

    };

    struct MOP;

    struct UopAsm: AsmWorker{
        ///// meta data
        MOP*               _mopMaster = nullptr;
        std::vector<token> _uopTokens;
        std::string        _uopName;
        int                _uopIdx = -1;
        ///// asm worker

        UopAsm(MOP* master,
               std::vector<token> tokens,
               std::string uopName, int uopIdx);
        void doAsm();


    };

    //// <5-u> <1100110>    <----- //// left for uop //// right for mop

    struct MOP{
        ///// meta data at start
        InstrRepo* _master = nullptr;
        std::vector<token>     _masterTokens; ///// master of all token
        std::string _mopName;
        int _mopIdx = -1;
        ///// after interpret and fetch
        std::vector<token>     _opcodeTokens;
        std::vector<token>     _masterUopTokens;
        std::string            _flattedInstr; ///// fill when flattendMop
        std::vector<RegIdxAsm> _srcRegAsms;
        std::vector<RegIdxAsm> _desRegAsms;
        ImmAsm                 _immAsm;
        std::vector<UopAsm>    _uops;

        MOP(InstrRepo* master,
            std::vector<token>& masterTokens,
            std::string mopName,
            int mopIdx);

        void        interpretMasterToken();
        void        createUop(std::vector<token>& uopTokens, const std::string& uopName);
        void        assignMaster(); //// assign master to all node
        void        flattenMop();
        [[nodiscard]]
        std::string getFlattenUop() const{return _flattedInstr;}
        [[nodiscard]]
        int         getAmtUop() const{return _uops.size();}
        void        doAllAsm();
        void        unsetUnusedReg(const bool* eff, int size, bool isSrc);
    };

}

#endif //INSTRELE_H
