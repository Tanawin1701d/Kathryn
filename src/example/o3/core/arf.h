//
// Created by tanawin on 28/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ARF_H
#define KATHRYN_SRC_EXAMPLE_O3_ARF_H

#include "kathryn.h"
#include "slot_param.h"
#include "mpft.h"

namespace kathryn::o3{

    static int ARF_MIS_PRIORITY = DEFAULT_UE_PRI_USER + 4;
    static int ARF_SUC_PRIORITY = DEFAULT_UE_PRI_USER + 3;
    static int ARF_REN_PRIORITY = DEFAULT_UE_PRI_USER + 2;
    static int ARF_COM_PRIORITY = DEFAULT_UE_PRI_USER + 1;

    struct RenameCmd{
        opr& ren_en;       // 1
        opr& ren_rrf_ptr;   // RRF_SEL
        opr& ren_arc_idx;   // REG_SEL
        opr& is_branch;
        opr& spec_tag;
    };

    struct RenamedData{
        opr& busy;
        opr& rrf_idx;
    };

    struct PreRenGrp{
        int idx = -1; //// -1 is for master
        m_expr(is_as_recv_grp, 1);
        WireSlot busy_temp  {sm_arf_busy};
        WireSlot rename_temp{sm_arf_renamed};


        /**
        *
        * for master table
        */
        void tied_to_master(RegSlot& busy_slot, RegSlot& rename_slot){
            busy_temp      = busy_slot;
            rename_temp    = rename_slot;
            busy_slot    <<= busy_temp;
            rename_slot  <<= rename_temp;
        }

        /**
         * for rcv table
         */

        ///// in normal case loop tied it to the system
        void tied_to_table(Table& busy_table, Table& rename_table){
            assert(idx != -1);
            busy_temp           = busy_table(idx);
            rename_temp         = rename_table(idx);
            busy_table(idx)   <<= busy_temp;
            rename_table(idx) <<= rename_temp;
        }

        void on_mis_pred(opr& mis_tag,
                       Table&    rcv_tab_busy,
                       Table&    rcv_tab_rename){
            SET_ASM_PRI_TO_MANUAL(ARF_MIS_PRIORITY);  ///CTRL ARF
            busy_temp   = rcv_tab_busy  [OH(mis_tag)].v();
            rename_temp = rcv_tab_rename[OH(mis_tag)].v();
            SET_ASM_PRI_TO_AUTO(); ///CTRL ARF
        }

        /////// system going to succcess
        void on_suc_pred(opr& suc_tag, PreRenGrp& master_ren_grp){
            SET_ASM_PRI_TO_MANUAL(ARF_SUC_PRIORITY); ///CTRL ARF
            zif(suc_tag.sl(idx) | (~is_as_recv_grp)){
                busy_temp   = master_ren_grp.busy_temp;
                rename_temp = master_ren_grp.rename_temp;
            }
            SET_ASM_PRI_TO_AUTO(); ///CTRL ARF
        }

        void commit_base(opr& com_en    , opr& com_rrf_ptr,
                        opr& com_arc_idx, RegSlot& rename_base){

            ///// busy doesnt have to be set if it unset already, it is ok!
            opr& com_entry_match =  (rename_base[com_arc_idx].v() == com_rrf_ptr);
            zif(com_en && com_entry_match){
                busy_temp[com_arc_idx] = 0;
            }

        }

        void on_commit(opr& com_en1    , opr& com_rrf_ptr1,
                      opr& com_arc_idx1,
                      opr& com_en2    , opr& com_rrf_ptr2,
                      opr& com_arc_idx2,
                      RegSlot& rename_reg){
            SET_ASM_PRI_TO_MANUAL(ARF_COM_PRIORITY); ///CTRL ARF
            commit_base(com_en1, com_rrf_ptr1, com_arc_idx1, rename_reg);
            commit_base(com_en2, com_rrf_ptr2, com_arc_idx2, rename_reg);
            SET_ASM_PRI_TO_AUTO(); ///CTRL ARF
        }

        void rename_base(RenameCmd& ren_cmd){
            zif(ren_cmd.ren_en){
                busy_temp[ren_cmd.ren_arc_idx]   = 1;
                rename_temp[ren_cmd.ren_arc_idx] = ren_cmd.ren_rrf_ptr;
            }
        }

        void on_rename(RenameCmd& ren_cmd1, RenameCmd& ren_cmd2, bool override = false){
            SET_ASM_PRI_TO_MANUAL(ARF_REN_PRIORITY); ///CTRL ARF
            if (override){
                rename_base(ren_cmd1);              //// order cannot be changed
                rename_base(ren_cmd2);
            }else{
                ////// the is_as_recv_grp is set from decode stage
                ////// it should be undone first
                opr& instr1WantThisSlotToRcv = (ren_cmd1.is_branch && ren_cmd1.spec_tag.sl(idx));
                opr& instr2WantThisSlotToRcv = (ren_cmd2.is_branch && ren_cmd2.spec_tag.sl(idx));
                ////// undo both instruction 1 and instruction 2
                opr& isAsRecvGrp_undo = (is_as_recv_grp &&
                                         (!(instr1WantThisSlotToRcv |
                                            instr2WantThisSlotToRcv))
                                         );
                ////// rename 1
                zif(~isAsRecvGrp_undo){
                    rename_base(ren_cmd1);
                }
                ////// rename 2 (if the first instruction is branch and this table is just rcv )
                opr& isAsRecvGrp_undo_only_second_instr = (isAsRecvGrp_undo |  instr1WantThisSlotToRcv);
                zif (~isAsRecvGrp_undo_only_second_instr){
                    rename_base(ren_cmd2);
                }
            }
            SET_ASM_PRI_TO_AUTO(); ///CTRL ARF
        }

    };


    ////// | rename <-> commit <-> success | miss_predict
    ////// mispredict copy the fix table to all table (master table include)
    ////// rename on all table that is free and master table (have the most priorty)
    ////// success copy the master to the success table (rename cannot occur at the same time with rename)
    ////// commit update all table that each element is busy except success table must use with after fixed table


    struct Arf{

        ////// rename table
        Table    busy        {sm_arf_busy, SPECTAG_LEN};
        Table    rename      {sm_arf_renamed, SPECTAG_LEN}; ////// row re recover tag col is reg
        RegSlot  busy_master  {sm_arf_busy};
        RegSlot  rename_master{sm_arf_renamed};
        PreRenGrp pre_ren_grp[SPECTAG_LEN];
        PreRenGrp pre_ren_master;
        ////// architecture data file
        RegSlot  arch_regs    {sm_arf_data};

        explicit Arf(Mpft& mpft){
            ////// reset the register
            busy        .make_reset_event(0);
            rename      .make_reset_event(0);
            busy_master  .make_reset_event(0);
            rename_master.make_reset_event(0);
            data_struct_prob_grp.arf_busy.init(&busy);      ///DC
            data_struct_prob_grp.arf_rename.init(&rename);  ///DC

            ////// initialize pre_ren_grp
            for(int i = 0; i < SPECTAG_LEN; i++){
                pre_ren_grp[i].idx = i;
                pre_ren_grp[i].is_as_recv_grp = mpft.is_used(i);
                pre_ren_grp[i].tied_to_table(busy, rename);
            }
            pre_ren_master.idx         = -1;
            pre_ren_master.is_as_recv_grp =  1;
            pre_ren_master.tied_to_master(busy_master, rename_master);
        }

        RenamedData get_renamed_data(opr& arch_idx){
            return {busy_master  [arch_idx].v(),
                    rename_master[arch_idx].v()};
        }

        opr& get_arf_data(opr& arch_idx){
            return arch_regs[arch_idx].v();
        }

        void update_arf_reg(opr& com_en, opr& com_arc_idx, opr& data){
            zif(com_en & (com_arc_idx != 0)){
                arch_regs[com_arc_idx] <<= data;
            }
        }
        void on_mis_pred(opr& mis_tag){
            for(int spec_idx = 0; spec_idx < SPECTAG_LEN; spec_idx++){
                pre_ren_grp[spec_idx].on_mis_pred(mis_tag, busy, rename);
            }
            pre_ren_master.on_mis_pred(mis_tag, busy, rename);
        }

        void on_suc_pred(opr& suc_tag){
            for (int spec_idx = 0; spec_idx < SPECTAG_LEN; spec_idx++){
                ///////// it must be data from pre_master because it can be occur with commit at the same time
                pre_ren_grp[spec_idx].on_suc_pred(suc_tag, pre_ren_master);
            }
        }

        void on_rename(RenameCmd& ren_cmd1, RenameCmd& ren_cmd2){
            for (int spec_idx = 0; spec_idx < SPECTAG_LEN; spec_idx++){
                pre_ren_grp[spec_idx].on_rename(ren_cmd1, ren_cmd2, false);
            }
            pre_ren_master.on_rename(ren_cmd1, ren_cmd2, true);
        }

        void on_commit(opr& com_en1    , opr& com_rrf_ptr1,
                      opr& com_arc_idx1, opr& com_data1  ,
                      opr& com_en2    , opr& com_rrf_ptr2,
                      opr& com_arc_idx2, opr& com_data2)
        {
            for (int spec_idx = 0; spec_idx < SPECTAG_LEN; spec_idx++){
                pre_ren_grp[spec_idx].on_commit(com_en1, com_rrf_ptr1, com_arc_idx1,
                                            com_en2, com_rrf_ptr2, com_arc_idx2,
                                            rename(spec_idx));
            }
            pre_ren_master.on_commit(com_en1, com_rrf_ptr1, com_arc_idx1,
                                  com_en2, com_rrf_ptr2, com_arc_idx2,
                                  rename_master);

            ////// does not need to update any priority//// order cannot be changed
            update_arf_reg(com_en1, com_arc_idx1, com_data1); ///   due to it contain commit eneable at the destination
            update_arf_reg(com_en2, com_arc_idx2, com_data2); ///

        }


        ////// | rename <-> commit <-> success | miss_predict
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ARF_H
