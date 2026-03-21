//
// Created by tanawin on 1/10/25.
//

#ifndef SRC_EXAMPLE_O3_EXECLDST_H
#define SRC_EXAMPLE_O3_EXECLDST_H

#include "kathryn.h"
#include "rob.h"
#include "stage_struct.h"
#include "store_buf.h"

namespace kathryn::o3{

    struct ExecLdSt: Module{

    LdStStage&   lss;
    RegArch&     reg_arch;
    BroadCast&   bc;
    Rob&         rob;
    RsvBase&     rsv;
    RegSlot&     ls_res; /// load store result stage
    ByPass&      bp;
    StoreBuf&    st_buf;
    PipSimProbe* psp1 = nullptr; ///DC
    ZyncSimProb* zsp  = nullptr; ///DC
    PipSimProbe* psp2 = nullptr; ///DC

    m_wire(dbg_effAddr, ADDR_LEN); ///DC


    explicit ExecLdSt(LdStStage& ldSt_stage,
                     RegArch&    reg_arch,
                     BroadCast&  bc,
                     Rob&        rob,
                     RsvBase&    rsv,
                     StoreBuf&   st_buf) :
        lss    (ldSt_stage),
        reg_arch(reg_arch),
        bc     (bc),
        rob    (rob),
        rsv    (rsv),
        ls_res  (lss.ls_res),
        bp     (reg_arch.bpp.add_by_pass_ele()),
        st_buf  (st_buf){

        rsv.sync.set_tag_tracker(rsv.exec_src);
        ///lss.sync2.set_tag_tracker(ls_res);
        //// set tag tracker
    }

    void set_sim_probe (PipSimProbe* in_psp){psp1 = in_psp;} ///DC
    void set_sim_probe2(PipSimProbe* in_psp){psp2 = in_psp;} ///DC
    void set_zync_prob (ZyncSimProb* in_zsp){zsp  = in_zsp;} ///DC

    void flow() override{

        ////// first stage

        RegSlot& src = rsv.exec_src;

        opr& is_load    = src(rd_use);
        opr& data      = src(phyIdx_2);
        opr& eff_addr   = src(phyIdx_1) + src(imm);

        dbg_effAddr = eff_addr; ///DC


        //////// operate the store buffer
        st_buf.flow();

        pip(rsv.sync){ try_init_probe(psp1); ///CTRL EXEC_LDST
            zyncc(lss.sync2, (is_load || (!st_buf.is_full()))){ try_init_probe(zsp) ///CTRL EXEC_LDST
                //////assign ordinaty data to next stage rrftag. rd_ise. spec. spectag
                ls_res <<= src;
                auto[buf_found, buf_data] =  st_buf.search_newest(eff_addr);
                ls_res(st_buf_data) <<= buf_data;
                ls_res(st_buf_hit)  <<= buf_found;
                ////// assign specific role
                zif(is_load){ /// try to read data from memory
                    //// read has more priority
                    SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);
                    lss.dmem_we     = 0;
                    lss.dmem_rwaddr = eff_addr;
                    SET_ASM_PRI_TO_AUTO();
                }zelse{ /// store data into buffer
                    st_buf.on_new_entry(src, data, eff_addr); /// store on buffer
                }
            }
        }
        //////// second stage
        opr& resolved_data = mux(ls_res(st_buf_hit), ls_res(st_buf_data), lss.dmem_rdata);
        bp.add_src(ls_res(rrftag), resolved_data);

        pip(lss.sync2){ try_init_probe(psp2) ///CTRL EXEC_LDST
            rob.on_write_back(ls_res(rrftag));
            zif(ls_res(rd_use)){ ///// it is a load data
                reg_arch.rrf.on_wback(ls_res(rrftag), resolved_data);
                reg_arch.bpp.do_by_pass(bp);
            }
        }
    }
    };

}

#endif //SRC_EXAMPLE_O3_EXECLDST_H