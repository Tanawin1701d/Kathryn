//
// Created by tanawin on 25/3/2025.
//

#ifndef SRC_KRIDE_DATATYPE_H
#define SRC_KRIDE_DATATYPE_H

#include "kathryn.h"
#include "lib/hw/slot/slot.h"
#include "krideParam.h"
#include "lib/hw/slot/table.h"

namespace kathryn{

    template <typename T>
    using Ref = std::reference_wrapper<T>;


    extern RowMeta instrMeta;   //// instruction meta data
    extern RowMeta intDecMeta, rrfMeta, memMeta, mdMeta; ///////// decoder for each aspect
    extern RowMeta joinDecMeta, specGenMeta; /////// ^---pooled decoder and speculative tag generator
    extern RowMeta RsvEntryMeta; ////// REM base
    extern RowMeta intREM, mulREM, brREM, ldstREM; ////// REM reservation element meta data
    extern RowMeta iMem, dReadMem, dWriteMem; ///// memory acccess
    extern RowMeta stBufByPass;         ////// store buffer
    extern RowMeta ldstSC_base, ldstSC; ////// SC = start commiting
    extern RowMeta execRobMeta, dpRobAllocMeta;
    extern RowMeta robEM; //// reorder buffer entry metaData


    struct D_EXEC_BRANCH{
        mWire(sucPred      , 1);
        mWire(misPred      , 1);
        mWire(rrfIdxRcv    , RRF_SEL);
        mWire(specTagRcvIdx, SPEC_TAG_SEL);
        mWire(specTagRcv   , SPEC_TAG_LEN); //// RCV means recovery ///// it may be more than one tag to rcv in each data

    };

    struct D_DISPATCH{
        mWire(rmAmt, RRF_SEL+1);
        mWire(specTagPoolReq, SPEC_TAG_LEN);

        ///////// TODO assign this signal
        mWire(amtRrfFree, RRF_SEL);

        Operable& isRrfFull() {return amtRrfFree == 0;}

    };

    struct D_INSTR{

        RegSlot fetchMeta;
        D_INSTR(): fetchMeta(instrMeta){}
    };

    struct D_DECODE{
        RegSlot decs[2];
        expression isBranches[2];
        RegSlot spGen;

        D_DECODE(): decs{RegSlot(joinDecMeta),RegSlot(joinDecMeta)},
                    isBranches{expression(1), expression(1)},
                    spGen(specGenMeta){}

        void decode(D_INSTR& fetchMeta, int decIdx); //// TODO decode the fucking instruction
    };


    struct D_IO_RSV{
        mWire(allocatable, 1);
        Candidate issueCd; ///// issue candidate
        Slot      issueBuf;
    };

    struct D_MEM{

        WireSlot ins, dataR, dataW; ///// intruction Read mem

        D_MEM():ins(iMem),
                dataR(dReadMem),
                dataW(dWriteMem){}

    };

    struct D_STB{
        mWire(full, 1);
        WireSlot ldBp; ////// load bypass
        D_STB(): ldBp(stBufByPass){}

    };

    struct D_LDST{
        RegSlot cmSlot; ///// commit slot
        D_LDST(): cmSlot(ldstSC){}
    };

    struct Rob;
    struct D_ROB{
        mWire(dpPtr, RRF_SEL); //// TODO assign this from somewhere
        WireSlot bcCommit1;
        WireSlot bcCommit2;

        mWire(commit1, 1);
        mWire(commit2, 1);
        mWire(commitAmt, 2);

        Rob* rob;

        D_ROB():
        bcCommit1(robEM, -1),
        bcCommit2(robEM, -1){}

    };

    struct Rrf;

    struct D_ALL{
        D_INSTR       fet;
        D_DECODE      dcd;
        D_DISPATCH    dis;
        D_EXEC_BRANCH exb;
       /// D_COMMIT      com;
        D_IO_RSV      rsvInt[2];
        D_IO_RSV      rsvMul;
        D_IO_RSV      rsvBranch;
        D_IO_RSV      rsvLstd;
        D_MEM         mem;
        D_STB         stb;
        D_LDST        ldst;
        D_ROB         rob;
        Rrf*          rrf = nullptr;

        //////// should i invalidate
        Operable& shouldInv(Operable& listenerSpecTag){
            assert(exb.specTagRcv.getOperableSlice().getSize() == listenerSpecTag.getOperableSlice().getSize());
            return (listenerSpecTag & exb.specTagRcv) == 1;
        }

        Operable& shouldInv(Slot& sl){
            return shouldInv(sl.at("specTag"));
        }

    };





}

#endif //SRC_KRIDE_DATATYPE_H
