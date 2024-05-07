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
            mReg(valid, 1);
            mReg(idx  , REG_IDX);
            mReg(val  , XLEN);

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
            mReg(isUopUse  , 1);
            mReg(isMemLoad , 1); ///// else is store
            mReg(size      , 2); //// 00 -> 8, 01 -> 16, 10 -> 32, 11-> reserve
            mReg(extendMode, 1); //// 0 zero extend 1 unsign Extend

            void set(bool isLoad, Operable& needSize, Operable& needExtendMode){
                isUopUse <<= 1;
                isMemLoad <<= isLoad;
                size <<= needSize;
                extendMode <<= needExtendMode;
            }

            void reset(){
                isUopUse <<= 0;
            }

        };

        struct aluUop{
            mReg(isUopUse           , 1);
            mReg(isAdd              , 1);
            mReg(isSub              , 1);
            mReg(isXor              , 1);
            mReg(isOr               , 1);
            mReg(isAnd              , 1);
            mReg(isCmpLessThanSign  , 1);
            mReg(isCmpLessThanUSign , 1);
            mReg(isShiftLeftLogical , 1);
            mReg(isShiftRightLogical, 1);
            mReg(isShiftRightArith  , 1);

            void reset(){
                isUopUse <<= 0;
            }
        };

        struct jumpUop{
            mReg(isUopUse  , 1);
            mReg(isJalR    , 1);
            mReg(isJal     , 1);

            mReg(extendMode, 1); //// 0 sign zero extend 1 unsigned
            mReg(isEq      , 1);
            mReg(isNEq     , 1);
            mReg(isLt      , 1);
            mReg(isGe      , 1);

            void reset(){
                isUopUse <<= 0;
            }

        };

        struct ldPc{
            mReg(isUopUse  , 1);
            mReg(needPc, 1); ///// read pc and m it extend

            void reset(){
                isUopUse <<= 0;
            }

        };

        struct UOp{
            RegEle  regData[RS_count];
            mReg(pc, XLEN);
            mReg(nextPc, XLEN);
            /** op code*/
            lsUop   opLs;
            aluUop  opAlu;
            jumpUop opCtrlFlow;
            ldPc    opLdPc;

            void reset(){
                opLs.reset();
                opLdPc.reset();
                opCtrlFlow.reset();
                opAlu.reset();
            }

        };

        struct FETCH_DATA{
            mReg(fetch_pc    , MEM_ADDR_IDX);
            mReg(fetch_nextpc, MEM_ADDR_IDX);
            mReg(fetch_instr, XLEN);
        };

        struct BYPASS_DATA{
            mWire(idx, REG_IDX);
            mWire(value, XLEN);
        };

    }

}

#endif //KATHRYN_ELEMENT_H
