//
// Created by tanawin on 3/4/2567.
//

#ifndef KATHRYN_FETCH_H
#define KATHRYN_FETCH_H

#include "kathryn.h"
#include "example/riscv/sub_system/storage_mgm.h"
#include "example/riscv/parameter.h"
#include "example/riscv/element.h"

namespace kathryn::riscv {

        struct Fetch {

            /////// debugger
            ZyncSimProb zync_sim_prob;
            PipSimProbe pip_sim_probe;

            CORE_DATA& cd;
            StorageMgmt& storage_mgmt;

            m_wire(read_en, 1);
            m_wire(par_check, 1);
            Operable&    read_fin;
            ////FlowBlockBase* fetch_block = nullptr;

            explicit Fetch(CORE_DATA& core_data, StorageMgmt& mem_mgmt):
            cd(core_data),
            storage_mgmt(mem_mgmt),
            read_fin(storage_mgmt.add_reader(read_en,core_data.pc(MEM_ADDR_SL))){
                cd.pc.make_reset_event();
            }


            void flow(){

                SyncMeta& sync_dec = cd.dc.sync;
                read_en = sync_dec._syncSlaveReady;

                pip(cd.ft.sync){auto_sync  init_probe(pip_sim_probe);
                    zyncc(cd.dc.sync, read_fin){ init_probe(zync_sim_prob);
                    /** fetch data is shared among fetch and decoder
                     ** we must m sure it is ready to recv
                     * */
                    // cd.pc.as_output_glob("cur_pc");
                    // m_wire(dbg_st_fetch, 1);
                    // dbg_st_fetch = 1;
                    // dbg_st_fetch.as_output_glob("st_fetch");

                    cd.ft.fetch_instr  <<= storage_mgmt.read_output;
                    cd.ft.fetch_pc     <<= cd.pc;
                    cd.ft.fetch_nextpc <<= cd.pc + 4;
                    cd.pc              <<= cd.pc + 4;
                }
                }
            }
        };
}

#endif //KATHRYN_FETCH_H
