//
// Created by tanawin on 8/4/2567.
//

#ifndef KATHRYN_WRITEBACK_H
#define KATHRYN_WRITEBACK_H

#include "kathryn.h"
#include "example/riscv/element.h"
#include "example/riscv/sub_system/storage_mgm.h"

namespace kathryn::riscv{

        struct WriteBack{
            CORE_DATA& cd;
            PipSimProbe pip_sim_probe;
        public:

            WriteBack(CORE_DATA& core_data): cd(core_data){}

            void flow(MemBlock& reg_file){

                OPR_HW& des_reg = cd.ex.wb_data;
                des_reg.idx  .as_output_glob("write_idx");
                des_reg.data .as_output_glob("write_data");
                des_reg.valid.as_output_glob("valid");


                pip(cd.wb.sync){ init_probe(pip_sim_probe);
                    zif((des_reg.valid) && (des_reg.idx != 0)) {
                        reg_file[des_reg.idx] <<= des_reg.data;
                        cd.bp.idx = des_reg.idx;
                        cd.bp.value = des_reg.data;

                        // m_wire(dbg_st_wb, 1);
                        // dbg_st_wb = 1;
                        // dbg_st_wb.as_output_glob("st_wb");

                    }
                }
            }
        };
}

#endif //KATHRYN_WRITEBACK_H
