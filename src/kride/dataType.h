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


    extern RowMeta instrMeta;
    extern RowMeta intDecMeta, rrfMeta, memMeta, mdMeta;
    extern RowMeta joinDecMeta, specGenMeta;
    extern RowMeta RsvEntryMeta;
    extern RowMeta intREM, mulREM, brREM, ldstREM;

    struct D_EXEC_BRANCH{
        mWire(sucPred      , 1);
        mWire(misPred      , 1);
        mWire(rrfIdxRcv    , RRF_SEL);
        mWire(specTagRcvIdx, SPEC_TAG_SEL);
        mWire(specTagRcv   , SPEC_TAG_LEN); //// RCV means recovery ///// it may be more than one tag to rcv in each data

    };


    struct D_COMMIT{
        mWire(comAmt        , RRF_SEL+1);
        mWire(nxtCommitIdx  , RRF_SEL+1);

        mWire(commitAddr0   , ARF_SEL);
        mWire(commitAddr1   , ARF_SEL);


    };


    struct D_DISPATCH{
        mWire(rmAmt, RRF_SEL+1);
        mWire(specTagPoolReq, SPEC_TAG_LEN);

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
        Candidate issueCand;
    };

    struct Rrf;
    struct D_ALL{
        D_INSTR       fet;
        D_DECODE      dcd;
        D_DISPATCH    dis;
        D_EXEC_BRANCH exb;
        D_COMMIT      com;
        D_IO_RSV      rsvInt[2];
        D_IO_RSV      rsvMul;
        D_IO_RSV      rsvBranch;
        D_IO_RSV      rsvLstd;
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
