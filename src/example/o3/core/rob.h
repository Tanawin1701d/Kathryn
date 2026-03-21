//
// Created by tanawin on 30/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ROB_H
#define KATHRYN_SRC_EXAMPLE_O3_ROB_H

#include "stage_struct.h"

namespace kathryn::o3{

    struct StoreBuf;

    struct Rob: Module{
        Table _table;

        m_wire(com1Status, 1      );
        m_wire(com2Status, 1      );
        m_reg (com_ptr    , RRF_SEL);
        m_wire(com_ptr2   , RRF_SEL);
        PipStage&  pm;
        WireSlot   com1Entry{_table[com_ptr  ].v()};
        WireSlot   com2Entry{_table[com_ptr+1].v()};
        WireSlot   selected_entry{sm_rob};
        RegArch&   reg_arch;
        StoreBuf&  store_buf;

        Rob(PipStage& pip_stage, RegArch& reg_arch,
            StoreBuf& store_buf):
            _table(sm_rob, RRF_NUM),
            pm(pip_stage),
            reg_arch(reg_arch),
            store_buf(store_buf){
            _table.make_col_reset_event(wb_fin, 0);
            _table.make_col_reset_event(is_branch, 0);
            com_ptr.make_reset_event();
            data_struct_prob_grp.commit.init(&_table); ///DC
        }

        opr& get_com_ptr(){ return com_ptr;}

        WireSlot& get_branch_update_entry(){ return selected_entry;}

        void flow() override;

        void on_dispatch(opr& idx, RegSlot& dp_value, RegSlot& dp_share_val){
            opr& opc = dp_value(inst)(0, 7);
            _table[idx](wb_fin) <<= 0;
            _table[idx](store_bit) <<= (opc == RV32_STORE);
            _table[idx] <<= dp_value;  //// s_branch, rd_use, rd_idx
            _table[idx] <<= dp_share_val; ///  bhr, pc
        }

        void on_write_back(opr& idx){
            _table[idx](wb_fin) <<= 1;
        }
    };



}
#endif //KATHRYN_SRC_EXAMPLE_O3_ROB_H
