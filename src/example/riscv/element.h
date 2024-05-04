//
// Created by tanawin on 5/4/2567.
//

#ifndef KATHRYN_ELEMENT_H
#define KATHRYN_ELEMENT_H


#include "kathryn.h"
#include "parameter.h"


namespace kathryn{

    namespace riscv{

        enum RISCV_REGTYPE{
            RS_des   = 0,
            RS_1     = 1,
            RS_2     = 2,
            RS_3     = 3,
            RS_count = 4
        };

        struct RegEle{
            makeReg(valid, 1);
            makeReg(idx  , REG_IDX);
            makeReg(val  , XLEN);

            void setFromRegFile(Operable& setIdx){
                valid <<= 0;
                idx   <<= setIdx;
            }

            void setFromImm(Operable& value){
                valid <<= 1;
                idx   <<= 0;
                val   <<= value;
            }

            void reset(){
                valid  <<= 0;
                idx   <<= 0;
            }

            void setZero(){
                valid <<= 1;
                idx   <<= 0;
                val   <<= 0;
            }

            RegEle& operator <<=(const RegEle& rhs){
                valid <<= rhs.valid;
                idx   <<= rhs.idx;
                val   <<= rhs.val; return *this;
            }

        };

        struct lsUop{
            makeReg(isUopUse  , 1);
            makeReg(isMemLoad , 1); ///// else is store
            makeReg(size      , 2); //// 00 -> 8, 01 -> 16, 10 -> 32, 11-> reserve
            makeReg(extendMode, 1); //// 0 zero extend 1 unsign Extend

            void set(bool isLoad, Operable& needSize, Operable& needExtendMode){
                isUopUse <<= 1;
                isMemLoad <<= isLoad;
                size <<= needSize;
                extendMode <<= needExtendMode;
            }

            void reset(){
                isUopUse <<= 0;
            }

            lsUop& operator <<=(const lsUop& rhs){
                isUopUse    <<= rhs.isUopUse;
                isMemLoad   <<= rhs.isMemLoad;
                size        <<= rhs.size;
                extendMode  <<= rhs.extendMode; return *this;

            }
        };

        struct aluUop{
            makeReg(isUopUse           , 1);
            makeReg(isAdd              , 1);
            makeReg(isSub              , 1);
            makeReg(isXor              , 1);
            makeReg(isOr               , 1);
            makeReg(isAnd              , 1);
            makeReg(isCmpLessThanSign  , 1);
            makeReg(isCmpLessThanUSign , 1);
            makeReg(isShiftLeftLogical , 1);
            makeReg(isShiftRightLogical, 1);
            makeReg(isShiftRightArith  , 1);

            void reset(){
                isUopUse <<= 0;
            }

            aluUop& operator <<= (const aluUop& rhs){
                isUopUse              <<= rhs.isUopUse;
                isAdd                 <<= rhs.isAdd;
                isSub                 <<= rhs.isSub;
                isXor                 <<= rhs.isXor;
                isOr                  <<= rhs.isOr;
                isAnd                 <<= rhs.isAnd;
                isCmpLessThanSign     <<= rhs.isCmpLessThanSign;
                isCmpLessThanUSign    <<= rhs.isCmpLessThanUSign;
                isShiftLeftLogical    <<= rhs.isShiftLeftLogical;
                isShiftRightLogical   <<= rhs.isShiftRightLogical;
                isShiftRightArith     <<= rhs.isShiftRightArith; return *this;
            }
        };

        struct jumpUop{
            makeReg(isUopUse  , 1);
            makeReg(isJalR    , 1);
            makeReg(isJal     , 1);

            makeReg(extendMode, 1); //// 0 sign zero extend 1 unsigned
            makeReg(isEq      , 1);
            makeReg(isNEq     , 1);
            makeReg(isLt      , 1);
            makeReg(isGe      , 1);

            void reset(){
                isUopUse <<= 0;
            }

            jumpUop& operator <<= (const jumpUop& rhs){
                isUopUse   <<= rhs.isUopUse;
                isJalR     <<= rhs.isJalR;
                isJal      <<= rhs.isJal;
                extendMode <<= rhs.extendMode;
                isEq       <<= rhs.isEq;
                isNEq      <<= rhs.isNEq;
                isLt       <<= rhs.isLt;
                isGe       <<= rhs.isGe; return *this;
            }

        };

        struct ldPc{
            makeReg(isUopUse  , 1);
            makeReg(needPc, 1); ///// read pc and make it extend

            void reset(){
                isUopUse <<= 0;
            }

            ldPc& operator <<= (const ldPc& rhs){
                isUopUse   <<=  rhs.isUopUse;
                needPc     <<=  rhs.needPc; return *this;
            }
        };

        struct UOp{
            RegEle  regData[RS_count];
            makeReg(pc, XLEN);
            makeReg(nextPc, XLEN);
            /** op code*/
            lsUop   opLs;
            aluUop  opAlu;
            jumpUop opCtrlFlow;
            ldPc    opLdPc;

            UOp& operator <<= (const UOp& rhs){
                for (int i = 0; i < RS_count; i++){
                    regData[i] <<= rhs.regData[i];
                }
                pc             <<= rhs.pc;
                nextPc         <<= rhs.pc;
                opLs           <<= rhs.opLs;
                opAlu          <<= rhs.opAlu;
                opCtrlFlow     <<= rhs.opCtrlFlow;
                opLdPc         <<= rhs.opLdPc; return *this;
            }

        };

        struct FETCH_DATA{
            makeReg(fetch_pc    , MEM_ADDR_IDX);
            makeReg(fetch_nextpc, MEM_ADDR_IDX);
            makeReg(fetch_instr, XLEN);
        };

        struct BYPASS_DATA{
            makeWire(idx, REG_IDX);
            makeWire(value, XLEN);
        };

    }

}

#endif //KATHRYN_ELEMENT_H
