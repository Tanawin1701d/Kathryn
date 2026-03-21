//
// Created by tanawin on 24/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
#define KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H

#include "parameter.h"
#include "slot_param.h"

#include "tag_gen.h"
#include "mpft.h"

#include "arf.h"
#include "broad_cast.h"
#include "rrf.h"
#include "sync_meta_pip.h"
#include "isa_param.h"

namespace kathryn::o3{

    struct FetchStage{
        m_reg (cur_pc, ADDR_LEN);
        m_wire(i_mem0, DATA_LEN);
        m_wire(i_mem1, DATA_LEN);
        m_wire(i_mem2, DATA_LEN);
        m_wire(i_mem3, DATA_LEN);

        SlotMeta meta{sm_fetch};
        RegSlot  raw {sm_fetch};

        SyncMeta sync    {"fetch_sync"}; ///CTRL FETCH


        FetchStage(){
            cur_pc.as_output_glob("cur_pc");
            i_mem0.as_input_glob ("i_mem0");
            i_mem1.as_input_glob ("i_mem1");
            i_mem2.as_input_glob ("i_mem2");
            i_mem3.as_input_glob ("i_mem3");

        }

        void inc_pc(opr& next_pc, bool is_mis_pred = false){
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER + is_mis_pred);
            cur_pc <<= next_pc;
            SET_ASM_PRI_TO_AUTO();
        }
    };

    struct DecodeStage{
        SlotMeta shared_meta  {sm_dec_shard};
        SlotMeta decoded_meta {sm_dec_base };

        RegSlot   dcd1    {decoded_meta};
        RegSlot   dcd2    {decoded_meta};
        WireSlot  dcw1    {decoded_meta};
        WireSlot  dcw2    {decoded_meta};
        RegSlot  dcd_shared{shared_meta};

        SyncMeta sync    {"decode_sync"}; ///CTRL DECODE

        Operable& get_is_aloc_rsv(RegSlot& dcd){ return dcw1(rsIdx_1); }

    };

    struct LdStStage{
        m_wire(dmem_rdata, DATA_LEN);
        m_wire(dmem_we , 1); ///CTRL GROB
        m_wire(dmem_rwaddr, ADDR_LEN); //// must mux with reading
        m_wire(dmem_wdata, DATA_LEN);
        RegSlot ls_res {sm_ld_st};
        SyncPip  sync2 {"ld_st_last_sync"}; ///CTRL EXEC_LDST

        LdStStage(){
            dmem_rdata  .as_input_glob ("dmem_rdata");
            dmem_we     .as_output_glob("dmem_we"); ///CTRL GROB
            dmem_rwaddr .as_output_glob("dmem_rwaddr");
            dmem_wdata  .as_output_glob("dmem_wdata");


        }

    };

    struct ByPass{
        int bp_idx = -1;
        m_wire(valid, 1);
        m_wire(rrf_idx, RRF_SEL);
        m_wire(val, DATA_LEN);

        ByPass(int bp_idx):bp_idx(bp_idx){}

        void try_assign_by_pass(Operable& des_ident, Reg& des_val){
            zif(valid && (des_ident == rrf_idx)){
                des_val <<= val;
            }
        }

        void add_src(opr& in_rrf_idx, opr& in_val){
            rrf_idx = in_rrf_idx;
            val    = in_val;
        }

    };

    struct Rsvs;
    struct ByPassPool{

        std::vector<ByPass*>  _bps;
        Rsvs*                 _rsvs = nullptr;

        ByPass& add_by_pass_ele(){
            _bps.emplace_back(new ByPass(_bps.size()));
            return **_bps.rbegin();
        }

        ~ByPassPool(){ for (ByPass* bp: _bps){delete bp;}}

        void add_rsvs(Rsvs* rsvs){
            _rsvs = rsvs;
        }

        opr& is_by_passing(opr& rrf_idx){
            opr* result = &(_bps[0]->valid & (rrf_idx == _bps[0]->rrf_idx));
            for (int i = 1; i < _bps.size(); i++){
                result = &((*result) | (_bps[i]->valid & (rrf_idx == _bps[i]->rrf_idx)));
            }
            return *result;
        }

        void ass_by_pass_data(Wire& des_wire, opr& rrf_idx){
            for (ByPass* bp : _bps){
                zif(bp->valid && (bp->rrf_idx == rrf_idx)){
                    des_wire = bp->val;
                }
            }
        }

        void do_by_pass(ByPass& bp);

    };



    struct TagMgmt{
        BroadCast bc;
        TagGen    tag_gen{bc};
        Mpft      mpft;


    };

    struct RegArch{
        Arf arf;
        Rrf rrf;
        ByPassPool bpp;

        RegArch(Mpft& mpft): arf(mpft){}
    };

    struct PipStage{
        FetchStage  ft;
        DecodeStage dc;
        LdStStage   ld_st;

        SyncMeta sync_dp    {"disp_sync"}; ///CTRL DECODE
        SyncMeta sync_rs    {"rsv_sync"}; ///CTRL DISPATCH

        SyncMeta sync_cm    {"commit_sync"}; ///CTRL ROB


        void on_mis_pred(){
            ////// kill the in-order stage
            ft.sync.kill_slave(true); ///CTRL FETCH
            dc.sync.kill_slave(true); ///CTRL DECODE
            sync_dp.kill_slave(true); ///CTRL DISPATCH
            sync_cm.hold_slave();     ///CTRL ROB



            ////// kill the out-of-order exec Unit stage
            //sync_ex1  .kill_if_tag_met(true, fix_tag);
            //sync_ex2  .kill_if_tag_met(true, fix_tag);
            //sync_mul  .kill_if_tag_met(true, fix_tag);
            //ld_st.sync1.kill_if_tag_met(true, fix_tag);
            //ld_st.sync2.kill_if_tag_met(true, fix_tag);
            ////// hold reservation station to exection unit
            // sync_ex1  .hold_master();
            // sync_ex2  .hold_master();
            // sync_mul  .hold_master();
            // sync_br   .hold_master();
            // ld_st.sync1.hold_master();
            ///ld_st .sync2.hold_master(); //// because the master is not reservation station
            ////// hold commit to not


        }
        void on_suc_pred(){
            dc.sync.hold_master(); ///CTRL DECODE //// hold fetch <-> decode
            sync_dp.hold_master(); ///CTRL DISPATCH //// hold decode <-> dispatch to generate tag, but allowing system to enter decode state
            sync_rs.hold_master(); ///CTRL RSV_SHARED //// hold dispatch <-> reservation station
        }


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_STAGEPARAM_H
