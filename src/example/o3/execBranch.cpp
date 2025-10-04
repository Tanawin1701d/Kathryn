//
// Created by tanawin on 4/10/25.
//

#include "execBranch.h"

namespace kathryn::o3{

    void BranchExec::onMisPred(opr& misTag){

        opr& fixTag = tagMgmt.mpft.getFixTag(OH(misTag));
        ////// kill the pipeline stage
        pm.ft.sync.killSlave();
        pm.dc.sync.killSlave();
        pm.ds.sync.killSlave();
        pm.ex.sync.killSlave(fixTag);
        ////// do recovery on the system
        tagMgmt.mpft.onMissPred();
        tagMgmt.tagGen.onMisPred();
        regArch.arf.onMisPred();
        regArch.rob.onMisPred();
        regArch.rrf.onMisPred(src(rrftag),
            regArch.rob.getComPtr());

    }

    void BranchExec::onSucPred(){

    }




}