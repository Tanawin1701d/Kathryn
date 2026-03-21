//
// Created by tanawin on 4/10/25.
//

#include "exec_branch.h"
#include "dispatch.h"
#include "store_buf.h"



namespace kathryn::o3{

    void BranchExec::on_mis_pred(opr& fix_tag, opr& mis_tag, opr& fix_pc){
        ////// update the meta-data
        tag_mgmt.bc.mis = 1;  /// not count as control flow data because it is only inflight update meta-data
        ////// fetch update pc and gshare pred system
        pm.ft.inc_pc(fix_pc, true);
        /////// pipeline stage management
        pm  .on_mis_pred();
        /////// reservation station management
        rsvs.on_mis_pred(fix_tag);
        ////// do recovery on the tag system
        tag_mgmt.mpft  .on_mis_pred();
        tag_mgmt.tag_gen.on_mis_pred(mis_tag);
        ////// do recovery on register architecture
        reg_arch.arf.on_mis_pred(mis_tag);
        reg_arch.rrf.on_mis_pred(src(rrftag),
                              rob.get_com_ptr());
        ///// do recovery on store buffer
        st_buf.on_mis_pred(fix_tag);

    }

    void BranchExec::on_suc_pred(opr& fix_tag, opr& suc_tag){
        ////// update the meta-data
        tag_mgmt.bc.suc = 1; /// not count as control flow data because it is only inflight update meta-data
        /////// dp update the register
        disp_mod.on_suc_pred(suc_tag);
        /////// pipeline stage management
        pm.on_suc_pred();
        /////// reservation station management
        rsvs.on_suc_pred(suc_tag);
        //// do update the tag system
        tag_mgmt.mpft.on_pred_suc(suc_tag);
        tag_mgmt.tag_gen.on_suc_pred();
        //// do update the register architecture
        reg_arch.arf.on_suc_pred(suc_tag);
        //// do update the store buffer
        st_buf.on_suc_pred(suc_tag);

    }

}
