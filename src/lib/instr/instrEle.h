//
// Created by tanawin on 9/7/2024.
//

#ifndef INSTRELE_H
#define INSTRELE_H

#include<vector>
#include<cassert>
#include "model/hwComponent/abstract/Slice.h"
#include "model/hwComponent/register/register.h"


namespace kathryn{

    class InstrRepo;

    constexpr  int TOKEN_ASM_TYPE_IDX = 1; //// at splitedValue
    constexpr  char TOKEN_ASM_TYPE_REG_IDX  = 'r'; //// register idx
    constexpr  char TOKEN_ASM_TYPE_IMM_IDX  = 'i'; //// imm idx


    struct token{

        Slice sl;
        std::string value;
        std::vector<std::string> splitedValue;
        void finalToken();
    };

    ///// the component that used to be instruction and micro-op material
    struct AsmWorker{
        InstrRepo* _master = nullptr;
        virtual ~AsmWorker() = default;
        virtual void doAsm() = 0;
        void setMaster(InstrRepo* master){
            assert(_master != nullptr);
            _master = master;
        }
    };

    struct RegIdxAsm: AsmWorker{
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
        void doAsm() override;
    };

    struct ImmAsm: AsmWorker{
        constexpr int TOKEN_FILLA_IDX = 2;
        constexpr int TOKEN_FILLB_IDX = 3;
        std::vector<Slice> _srcSlices;
        std::vector<Slice> _desSlices;

        explicit ImmAsm() = default;
        void     addImmMeta(const token& tk);
        void     doAsm() override;

    };

    struct UopAsm;

    struct OpcodeAsm: AsmWorker{
        UopAsm*                  _uopMaster = nullptr;
        std::vector<Slice>       _srcSlices;
        std::vector<std::string> _expectValues;
        ////// op structure destination
        explicit    OpcodeAsm() = default;
        void        addOpMeta(const token& tk);
        void        setUopMaster(UopAsm* uopMaster);
        void        doAsm() override;
        std::string getSumOpcode() const;
    };

    struct UopAsm{
        ///// meta data
        InstrRepo*  _master = nullptr;
        std::string _uopName;
        int         _typeIdx  = -1; //// type of instruction
        int         _uopIdx   = -1; ///// micro op id
        std::vector<token>     _tokens;
        std::string _flattedOpcode;
        ///// asm worker
        std::vector<RegIdxAsm> _srcRegAsms;
        std::vector<RegIdxAsm> _desRegAsms;
        OpcodeAsm              _opAsm;
        ImmAsm                 _immAsm;

        UopAsm(InstrRepo* master,
               std::vector<token> tokens,std::string uopName,
               int typeIdx, int uopIdx);
        ///////// read token and build Asm worker
        void intepretToken();
        void assignMasterToAsm();
        void startGetSumOpcode();
        std::string getFlattenOpcode(){return _flattedOpcode;}
        ///////// do assignment to declare the state of this micro-op
        void doAllAsm();

    };

}

#endif //INSTRELE_H
