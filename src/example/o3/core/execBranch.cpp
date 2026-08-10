//
// Created by tanawin on 4/10/25.
//

#include "execBranch.h"
#include "dispatch.h"
#include "storeBuf.h"



namespace kathryn::o3{

    void BranchExec::onMisPred(opr& fixTag, opr& misTag, opr& fixPc){   ///CTRL_HC+DATA_HC EXEC_BRANCH
        ////// update the meta-data
        tagMgmt.bc.mis = 1;  // not count as control flow data because it is only inflight update meta-data   ///CTRL_DT EXEC_BRANCH
        ////// fetch update pc and gshare pred system
        pm.ft.incPc(fixPc, true);           ///DATA_HC EXEC_BRANCH
        /////// pipeline stage management
        pm  .onMisPred();                   ///CTRL_HC EXEC_BRANCH
        /////// reservation station management
        rsvs.onMisPred(fixTag);             ///CTRL_HC EXEC_BRANCH
        ////// do recovery on the tag system
        tagMgmt.mpft  .onMisPred();         ///CTRL_HC EXEC_BRANCH
        tagMgmt.tagGen.onMisPred(misTag);   ///CTRL_HC EXEC_BRANCH
        ////// do recovery on register architecture
        regArch.arf.onMisPred(misTag);            ///CTRL_HC EXEC_BRANCH
        regArch.rrf.onMisPred(src(rrftag),        ///CTRL_HC EXEC_BRANCH
                              rob.getComPtr());   ///CTRL_HC EXEC_BRANCH
        ///// do recovery on store buffer
        stBuf.onMisPred(fixTag);                  ///CTRL_HC EXEC_BRANCH

    }

    void BranchExec::onSucPred(opr& fixTag, opr& sucTag){   ///CTRL_HC EXEC_BRANCH
        ////// update the meta-data
        tagMgmt.bc.suc = 1; // not count as control flow data because it is only inflight update meta-data   ///CTRL_DT EXEC_BRANCH
        /////// dp update the register
        dispMod.onSucPred(sucTag);        ///CTRL_HC EXEC_BRANCH
        /////// pipeline stage management
        pm.onSucPred();                   ///CTRL_HC EXEC_BRANCH
        /////// reservation station management
        rsvs.onSucPred(sucTag);           ///CTRL_HC EXEC_BRANCH
        //// do update the tag system
        tagMgmt.mpft.onPredSuc(sucTag);   ///CTRL_HC EXEC_BRANCH
        tagMgmt.tagGen.onSucPred();       ///CTRL_HC EXEC_BRANCH
        //// do update the register architecture
        regArch.arf.onSucPred(sucTag);    ///CTRL_HC EXEC_BRANCH
        //// do update the store buffer
        stBuf.onSucPred(sucTag);          ///CTRL_HC EXEC_BRANCH

    }

}
