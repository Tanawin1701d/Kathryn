//
// Created by tanawin on 14/10/25.
//

#ifndef KATHRYN_O3SLOTRECODER_H
#define KATHRYN_O3SLOTRECODER_H

#include "kathryn.h"
#include "string"
#include "vector"

#include "example/o3/core/core.h"
#include "lib/instr/instr_base.h"
#include "sim/model_sim_engine/hw_collection/data_structure/slot/slot_prober.h"
#include "example/o3/sim_share/rec_pip_stage.h"


namespace kathryn::o3{

    class O3SlotRecorder{
    public:
        SlotWriter*  _slotWriter = nullptr;
        Core*        _core       = nullptr;
        PipStage*    _ps         = nullptr;

        bool is_last_cycle_mis_pred = false;
        bool is_last_cycle_suc_pred = false;

        ull  last_dispatch_ptr  = 0;
        bool is_last_cycle_disp1 = false;
        bool is_last_cycle_disp2 = false;


        O3SlotRecorder(SlotWriter*  slot_writer,
                       Core*        core):
        _slotWriter(slot_writer),
        _core(core),
        _ps(&_core->pm){
            assert(_slotWriter != nullptr);
        }



        void record_slot();

        /////// pipeline idle case writer
        bool write_slot_if_pip_idle(REC_PIP_STAGE stage_idx,
                                      PipSimProbe* pip_probe);
        bool write_slot_if_zync_stall(REC_PIP_STAGE stage_idx,
                                      ZyncSimProb* zync_sim_probe);
        //////// table writer
        void write_slot_if_table_change(REC_PIP_STAGE stage_idx,
                                    std::vector<SlotSimInfo64> change_rows,
                                    int row_lim_to_print_entire_row) const;

        ////// pipeline writer
        void write_mpft_slot();
        std::vector<std::string> get_arf_slot_val(RegSlot& busy_entry,
                                               RegSlot& rename_entry);
        void write_arf_slot ();
        void write_rrf_slot () const;

        void write_fetch_slot   ();
        void write_decode_slot  ();
        void write_dispatch_slot();

        std::pair<bool, std::vector<std::string>>
             write_rsv_slot      (RegSlot& entry);
        void write_rsv_basic_slot (Table& table);
        void write_rsv_alu_slot   (int idx, ORsv& orsv);
        void write_rsv_mul_slot   ();
        void write_rsv_branch_slot();
        void write_rsv_load_slot  ();

        std::pair<bool, std::vector<std::string>>
             write_st_buf_slot  (RegSlot& entry);
        void write_st_buf_table ();


        void write_issue_alu_slot   (int idx, ORsv& orsv, ZyncSimProb& z_issue_probe);
        void write_issue_mul_slot   ();
        void write_issue_branch_slot();
        void write_issue_ld_st_slot  ();

        void write_execute_basic     (RegSlot& src);
        void write_execute_ldst_basic (RegSlot& src);
        void write_execute_alu_slot   (int idx, ORsv& orsv, PipSimProbe& p_exec_probe);
        void write_execute_mul_slot   ();
        std::pair<bool, bool>
             write_execute_branch_slot();
        void write_execute_ld_st_slot  ();

        ////// {mis_pred, suc_pred}


        std::vector<std::string>
             write_rob_slot(ull rob_idx);
        void write_commit_slot();

        std::string translate_opcode(ull raw_instr);
        std::string translate_alu_op(ull alu_op_idx);


    };
}


#endif //KATHRYN_O3SLOTRECODER_H