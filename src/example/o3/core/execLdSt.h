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

    struct ExecLdSt: Module{   ///MD EXEC_LDST

    LdStStage&   lss;       ///CTRL_HC+DATA_HC EXEC_LDST
    RegArch&     regArch;   ///CTRL_HC+DATA_HC EXEC_LDST
    BroadCast&   bc;        ///CTRL_HC EXEC_LDST
    Rob&         rob;       ///CTRL_HC+DATA_HC EXEC_LDST
    RsvBase&     rsv;       ///CTRL_HC+DATA_HC EXEC_LDST
    RegSlot&     lsRes; // load store result stage   ///CTRL_HC+DATA_HC EXEC_LDST
    ByPass&      bp;   ///CTRL_HC+DATA_HC EXEC_LDST
    StoreBuf&    stBuf;   ///CTRL_HC+DATA_HC EXEC_LDST
    PipSimProbe* psp1 = nullptr; ///DC
    ZyncSimProb* zsp  = nullptr; ///DC
    PipSimProbe* psp2 = nullptr; ///DC

    mWire(dbg_effAddr, ADDR_LEN); ///DC


    explicit ExecLdSt(LdStStage& ldSt_stage,   ///CTRL_HC+DATA_HC EXEC_LDST
                     RegArch&    regArch,   ///CTRL_HC+DATA_HC EXEC_LDST
                     BroadCast&  bc,   ///CTRL_HC EXEC_LDST
                     Rob&        rob,   ///CTRL_HC+DATA_HC EXEC_LDST
                     RsvBase&    rsv,   ///CTRL_HC+DATA_HC EXEC_LDST
                     StoreBuf&   stBuf) :   ///CTRL_HC+DATA_HC EXEC_LDST
        lss    (ldSt_stage),   ///CTRL_HC+DATA_HC EXEC_LDST
        regArch(regArch),   ///CTRL_HC+DATA_HC EXEC_LDST
        bc     (bc),   ///CTRL_HC EXEC_LDST
        rob    (rob),   ///CTRL_HC+DATA_HC EXEC_LDST
        rsv    (rsv),   ///CTRL_HC+DATA_HC EXEC_LDST
        lsRes  (lss.lsRes),   ///CTRL_HC+DATA_HC EXEC_LDST
        bp     (regArch.bpp.addByPassEle()),   ///CTRL_HC+DATA_HC EXEC_LDST
        stBuf  (stBuf){   ///CTRL_HC+DATA_HC EXEC_LDST

        rsv.sync.setTagTracker(rsv.execSrc);   ///CTRL_HC EXEC_LDST
        ///lss.sync2.setTagTracker(lsRes);
        //// set tag tracker
    }

    void setSimProbe (PipSimProbe* in_psp){psp1 = in_psp;} ///DC
    void setSimProbe2(PipSimProbe* in_psp){psp2 = in_psp;} ///DC
    void setZyncProb (ZyncSimProb* in_zsp){zsp  = in_zsp;} ///DC

    void flow() override{   ///HLH EXEC_LDST

        ////// first stage

        RegSlot& src = rsv.execSrc;   ///CTRL_DT+DATA_DT EXEC_LDST

        opr& isLoad    = src(rdUse);   ///CTRL_DT EXEC_LDST
        opr& data      = src(phyIdx_2);   ///DATA_DT EXEC_LDST
        opr& effAddr   = src(phyIdx_1) + src(imm);   ///DATA_CL EXEC_LDST

        dbg_effAddr = effAddr; ///DC


        //////// operate the store buffer
        stBuf.flow();   ///CTRL_HC+DATA_HC EXEC_LDST

        pip(rsv.sync){ tryInitProbe(psp1);   ///CTRL_HWD+CTRL_CL EXEC_LDST
            zyncc(lss.sync2, (isLoad || (!stBuf.isFull()))){ tryInitProbe(zsp)   ///CTRL_HWD+CTRL_CL EXEC_LDST
                //////assign ordinaty data to next stage rrftag. rdIse. spec. spectag
                lsRes <<= src;   ///CTRL_DT+DATA_DT EXEC_LDST
                auto[buf_found, buf_data] =  stBuf.searchNewest(effAddr);   ///HLH EXEC_LDST
                lsRes(stBufData) <<= buf_data;   ///DATA_DT EXEC_LDST
                lsRes(stBufHit)  <<= buf_found;   ///CTRL_DT EXEC_LDST
                ////// assign specific role
                zif(isLoad){ // try to read data from memory   ///CTRL_CL EXEC_LDST
                    //// read has more priority
                    SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);   ///CTRL_CL EXEC_LDST
                    lss.dmem_we     = 0;   ///CTRL_DT EXEC_LDST
                    lss.dmem_rwaddr = effAddr;   ///DATA_DT EXEC_LDST
                    SET_ASM_PRI_TO_AUTO();   ///CTRL_CL EXEC_LDST
                }zelse{ // store data into buffer ///CTRL_CL EXEC_LDST
                    stBuf.onNewEntry(src, data, effAddr); // store on buffer   ///DATA_HC EXEC_LDST
                }
            }
        }
        //////// second stage
        opr& resolvedData = mux(lsRes(stBufHit), lsRes(stBufData), lss.dmem_rdata);   ///DATA_CL EXEC_LDST
        bp.addSrc(lsRes(rrftag), resolvedData);   ///CTRL_HC+DATA_HC EXEC_LDST

        pip(lss.sync2){ tryInitProbe(psp2)   ///CTRL_HWD+CTRL_CL EXEC_LDST
            rob.onWriteBack(lsRes(rrftag));   ///CTRL_HC EXEC_LDST
            zif(lsRes(rdUse)){ ///// it is a load data   ///CTRL_CL EXEC_LDST
                regArch.rrf.onWback(lsRes(rrftag), resolvedData);   ///CTRL_HC+DATA_HC EXEC_LDST
                regArch.bpp.doByPass(bp);   ///CTRL_HC+DATA_HC EXEC_LDST
            }
        }
    }
    };

}

#endif //SRC_EXAMPLE_O3_EXECLDST_H
