//
// Created by tanawin on 1/10/25.
//

#ifndef SRC_EXAMPLE_O3_EXECLDST_H
#define SRC_EXAMPLE_O3_EXECLDST_H

#include "kathryn.h"
#include "rob.h"
#include "stageStruct.h"
#include "storeBuf.h"

namespace kathryn::o3{

    struct ExecLdSt: Module{

    LdStStage&   lss;
    RegArch&     regArch;
    BroadCast&   bc;
    Rob&         rob;
    RsvBase&     rsv;
    RegSlot&     lsRes; /// load store result stage
    ByPass&      bp;
    StoreBuf&    stBuf;
    PipSimProbe* psp1 = nullptr; ///DC
    ZyncSimProb* zsp  = nullptr; ///DC
    PipSimProbe* psp2 = nullptr; ///DC

    mWire(dbg_effAddr, ADDR_LEN); ///DC


    explicit ExecLdSt(LdStStage& ldSt_stage,
                     RegArch&    regArch,
                     BroadCast&  bc,
                     Rob&        rob,
                     RsvBase&    rsv,
                     StoreBuf&   stBuf) :
        lss    (ldSt_stage),
        regArch(regArch),
        bc     (bc),
        rob    (rob),
        rsv    (rsv),
        lsRes  (lss.lsRes),
        bp     (regArch.bpp.addByPassEle()),
        stBuf  (stBuf){

        rsv.sync.setTagTracker(rsv.execSrc);
        ///lss.sync2.setTagTracker(lsRes);
        //// set tag tracker
    }

    void setSimProbe (PipSimProbe* inPsp){psp1 = inPsp;} ///DC
    void setSimProbe2(PipSimProbe* inPsp){psp2 = inPsp;} ///DC
    void setZyncProb (ZyncSimProb* inZsp){zsp  = inZsp;} ///DC

    void flow() override{

        ////// first stage

        RegSlot& src = rsv.execSrc;

        opr& isLoad    = src(rdUse);
        opr& data      = src(phyIdx_2);
        opr& effAddr   = src(phyIdx_1) + src(imm);

        dbg_effAddr = effAddr; ///DC


        //////// operate the store buffer
        stBuf.flow();

        pip(rsv.sync){ tryInitProbe(psp1); ///CTRL EXEC_LDST
            zyncc(lss.sync2, (isLoad || (!stBuf.isFull()))){ tryInitProbe(zsp) ///CTRL EXEC_LDST
                //////assign ordinaty data to next stage rrftag. rdIse. spec. spectag
                lsRes <<= src;
                auto[bufFound, bufData] =  stBuf.searchNewest(effAddr);
                lsRes(stBufData) <<= bufData;
                lsRes(stBufHit)  <<= bufFound;
                ////// assign specific role
                zif(isLoad){ /// try to read data from memory
                    //// read has more priority
                    SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);
                    lss.dmemWe     = 0;
                    lss.dmemRwaddr = effAddr;
                    SET_ASM_PRI_TO_AUTO();
                }zelse{ /// store data into buffer
                    stBuf.onNewEntry(src, data, effAddr); /// store on buffer
                }
            }
        }
        //////// second stage
        opr& resolvedData = mux(lsRes(stBufHit), lsRes(stBufData), lss.dmemRdata);
        bp.addSrc(lsRes(rrftag), resolvedData);

        pip(lss.sync2){ tryInitProbe(psp2) ///CTRL EXEC_LDST
            rob.onWriteBack(lsRes(rrftag));
            zif(lsRes(rdUse)){ ///// it is a load data
                regArch.rrf.onWback(lsRes(rrftag), resolvedData);
                regArch.bpp.doByPass(bp);
            }
        }
    }
    };

}

#endif //SRC_EXAMPLE_O3_EXECLDST_H