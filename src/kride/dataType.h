//
// Created by tanawin on 25/3/2025.
//

#ifndef SRC_KRIDE_DATATYPE_H
#define SRC_KRIDE_DATATYPE_H

#include "kathryn.h"
#include "lib/hw/slot/slot.h"
#include "krideParam.h"

namespace kathryn{

    template <typename T>
    using Ref = std::reference_wrapper<T>;


    extern RowMeta instrMeta;
    extern RowMeta intDecMeta, rrfMeta, memMeta, mdMeta;
    extern RowMeta joinDecMeta, specGenMeta;
    extern RowMeta OORsvEntry, intRsvEntry;

    struct D_EXEC_BRANCH{
        mWire(sucPred      , 1);
        mWire(misPred      , 1);
        mWire(rrfIdxRcv    , RRF_SEL);
        mWire(specTagRcvIdx, SPEC_TAG_SEL);
        mWire(specTagRcv   , SPEC_TAG_LEN); //// RCV means recovery
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


    struct D_ALL{
        D_INSTR       fet;
        D_DECODE      dcd;
        D_DISPATCH    dis;
        D_EXEC_BRANCH exb;
        D_COMMIT      com;

    };




}

#endif //SRC_KRIDE_DATATYPE_H
