//
// Created by tanawin on 4/10/25.
//

#include "execBranch.h"
#include "rsv.h"

namespace kathryn::o3{

    void BranchExec::onMisPred(opr& misTag, opr& fixPc){
        ////// update the meta-data
        tagMgmt.bc.mis = 1;
        opr& fixTag = tagMgmt.mpft.getFixTag(OH(misTag));
        pm.ft.curPc <<= fixPc;
        ////// kill the pipeline stage
        pm.ft.sync.killSlave();
        pm.dc.sync.killSlave();
        pm.ds.sync.killSlave();
        pm.ex.sync.killSlave(fixTag);
        ////// do recovery on the tag system
        tagMgmt.mpft.onMissPred();
        tagMgmt.tagGen.onMisPred();
        ////// do recovery on the reservation station
        for (RsvBase* rsv: rsvs){
            rsv->onMisPred(fixTag);
        }
        ////// do recovery on register architecture
        regArch.arf.onMisPred(misTag);
        regArch.rrf.onMisPred(src(rrftag),
        rob.getComPtr());
        ////// do hold the commit
        rob.onMisPred();

    }

    void BranchExec::onSucPred(opr& sucTag){
        tagMgmt.bc.suc = 1;
        //// stall the pipeline
        pm.dc.sync.holdSlave(); //// because mpft and tag generator must be hold
        pm.ds.sync.holdSlave();

        //// do update the tag system
        tagMgmt.mpft.onPredSuc(src(specTag));
        tagMgmt.tagGen.onSucPred();

        //// do update the reservartion stationo
        for (RsvBase* rsv: rsvs){
            rsv->onSucPred(sucTag);
        }

        //// do update the register architecture
        regArch.arf.onSucPred();

    }




}