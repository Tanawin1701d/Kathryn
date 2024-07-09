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

        explicit RegIdxAsm(token& tk);
        void doAsm() override;
    };

    struct ImmAsm: AsmWorker{
        constexpr int TOKEN_FILLA_IDX = 2;
        constexpr int TOKEN_FILLB_IDX = 3;
        std::vector<Slice> _srcSlices;
        std::vector<Slice> _desSlices;

        explicit ImmAsm() = default;
        void     addImmMeta(token& tk);
        void     doAsm() override;

    };

    struct OpcodeAsm: AsmWorker{
        std::vector<Slice>       _srcSlices;
        std::vector<std::string> _expectValues;
        ////// op structure destination
        explicit OpcodeAsm() = default;
        void     addOpMeta(token& tk);
        void     doAsm() override;
    };

    struct InstrType{
        ///// src instr
        std::vector<token>     _tokens;
        ///// asm worker
        std::vector<RegIdxAsm> _srcRegAsms;
        std::vector<RegIdxAsm> _desRegAsms;
        OpcodeAsm              _opAsm;
        ImmAsm                 _immAsm;

        InstrType(std::vector<token> tokens);
        ///////// read token and build Asm worker
        void intepretToken();
        ///////// do assignment to declare the state of this micro-op
        void doAllAsm();

    };

}

#endif //INSTRELE_H
