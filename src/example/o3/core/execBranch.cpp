//
// Created by tanawin on 4/10/25.
//

#include "fetch.h"
#include "execBranch.h"

#include "dispatch.h"
#include "rsv.h"
#include "rsvs.h"

namespace kathryn::o3{

    void BranchExec::onMisPred(opr& fixTag, opr& misTag, opr& fixPc){
        ////// update the meta-data
        tagMgmt.bc.mis = 1;
        ////// fetch update pc and gshare pred system
        fetchMod.onMisPred(misTag, fixPc);
        /////// pipeline stage management
        pm  .onMisPred(fixTag);
        /////// reservation station management
        rsvs.onMisPred(fixTag);
        ////// do recovery on the tag system
        tagMgmt.mpft  .onMissPred();
        tagMgmt.tagGen.onMisPred(misTag);
        ////// do recovery on register architecture
        regArch.arf.onMisPred(misTag);
        regArch.rrf.onMisPred(src(rrftag),
                              rob.getComPtr());

    }

    void BranchExec::onSucPred(opr& fixTag, opr& sucTag){
        ////// update the meta-data
        tagMgmt.bc.suc = 1;
        /////// fetch update gshare pred system
        fetchMod.onSucPred(sucTag);
        /////// dp update the register
        dispMod.onSucPred(sucTag);
        /////// pipeline stage management
        pm.onSucPred();
        /////// reservation station management
        rsvs.onSucPred(sucTag);
        //// do update the tag system
        tagMgmt.mpft.onPredSuc(src(specTag));
        tagMgmt.tagGen.onSucPred();
        //// do update the register architecture
        regArch.arf.onSucPred(sucTag);

    }

}
