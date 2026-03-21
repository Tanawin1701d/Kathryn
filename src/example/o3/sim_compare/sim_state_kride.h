//
// Created by tanawin on 24/12/25.
//

#ifndef KATHRYN_SIMSTATEKRIDE_H
#define KATHRYN_SIMSTATEKRIDE_H

#include "kathryn.h"
#include "sim_state.h"
#include "example/o3/core/core.h"

namespace kathryn::o3{


    struct SimStateKride : SimState{

        Core& _core;

        bool is_last_cycle_mis_pred = false;
        bool is_last_cycle_succ    = false;

        bool is_last_cycle_disp1 = false;
        bool is_last_cycle_disp2 = false;
        ull  last_dispatch_ptr  = 0;

        SimStateKride(Core& core):
            SimState(),
            _core(core){}

        pip_stat generate_pip_state(PipSimProbe* pip_probe,
                                 ZyncSimProb* zync_sim_probe);

        ///// rsv helper
        void assign_ele_if_there(ull& ele, RegSlot& val , const std::string& ele_name);
        void assignRSV_Entry (RSV_BASE_ENTRY& entry  , RegSlot& reg_slot);
        void assignRSV_Branch(RSV_BRANCH_ENTRY& entry, RegSlot& reg_slot);
        void assignRSV_Mul   (RSV_MUL_ENTRY& entry   , RegSlot& reg_slot);

        void assignEXEC_Entry (RSV_BASE_ENTRY& entry  , RegSlot& reg_slot);
        void assignEXEC_Branch(RSV_BRANCH_ENTRY& entry, RegSlot& reg_slot);
        void assignEXEC_Mul   (RSV_MUL_ENTRY& entry   , RegSlot& reg_slot);
        void assignEXEC_LDST (RSV_BASE_ENTRY& entry  , RegSlot& reg_slot) ;

        void assignARF_Table (int table_idx, RegSlot& busy_slot, RegSlot& rename_slot);

        void recruit_value() override;
        void recruit_next_cycle() override;

        void print_slot_window(SlotWriterBase& writer) override;

        void write_slot_if_table_change(SlotWriterBase& writer,
                                    REC_PIP_STAGE stage_idx,
                                    std::vector<SlotSimInfo64> change_rows,
                                    int row_lim_to_print_entire_row) const;


    };

}

#endif //KATHRYN_SIMSTATEKRIDE_H