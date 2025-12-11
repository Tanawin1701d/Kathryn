//
// Created by tanawin on 1/10/25.
//

#ifndef SRC_EXAMPLE_O3_EXECLDST_H
#define SRC_EXAMPLE_O3_EXECLDST_H

#include "alu.h"
#include "kathryn.h"
#include "rob.h"
#include "srcSel.h"
#include "stageStruct.h"
#include "storeBuf.h"
#include "model/hwCollection/dataStructure/mux/mux.h"

namespace kathryn::o3{

    struct ExecLdSt: Module{

    LdStStage&   lss;
    RegArch&     regArch;
    BroadCast&   bc;
    Rob&         rob;
    RegSlot&     src;
    RegSlot&     lsRes; /// load store result stage
    ByPass&      bp;
    StoreBuf&    stBuf;
    PipSimProbe* psp1 = nullptr;
    PipSimProbe* psp2 = nullptr;


    explicit ExecLdSt(LdStStage& ldSt_stage,
                     RegArch&    regArch,
                     BroadCast&  bc,
                     Rob&        rob,
                     RegSlot&    src,
                     StoreBuf&   stBuf) :
        lss(ldSt_stage),
        regArch(regArch),
        bc(bc),
        rob(rob),
        src(src),
        lsRes(lss.lsRes),
        bp(regArch.bpp.addByPassEle()),
        stBuf(stBuf){

        lss.sync.setTagTracker(src);
        lss.sync2.setTagTracker(lsRes);
        //// set tag tracker
    }

    void setSimProbe (PipSimProbe* in_psp){psp1 = in_psp;}
    void setSimProbe2(PipSimProbe* in_psp){psp2 = in_psp;}

    void flow() override{

        ////// first stage
        opr& isLoad    = src(rdUse);
        opr& data      = src(phyIdx_2);
        opr& effAddr   = src(phyIdx_1) + src(imm);

        pip(lss.sync){ tryInitProbe(psp1);
            zyncc(lss.sync2, (isLoad || (!stBuf.isFull()))){
                //////assign ordinaty data to next stage rrftag. rdIse. spec. spectag
                lsRes <<= src;
                zif(bc.checkIsSuc(src)){
                    lsRes(spec) <<= 0; /// on flight clean data
                }
                ////// assign specific role
                zif(isLoad){ /// try to read data from memory
                    auto[buf_found, buf_data] =  stBuf.searchNewest(effAddr);
                    lsRes(stBufData) <<= buf_data;
                    lsRes(stBufHit)  <<= buf_found;
                    //// read has more priority
                    SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);
                    lss.dmem_we     = 0;
                    lss.dmem_rwaddr = effAddr;
                    SET_ASM_PRI_TO_AUTO();
                }zelse{ /// store data into buffer
                    stBuf.onNewEntry(src, data, effAddr); /// store on buffer
                }
            }
        }
        //////// second stage
        opr& resolvedData = mux(lsRes(stBufHit), lss.dmem_rdata, lsRes(stBufData));
        bp.addSrc(src(rrftag), resolvedData);

        pip(lss.sync2){ tryInitProbe(psp2)
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