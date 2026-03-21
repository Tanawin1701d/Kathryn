//
// Created by tanawin on 1/4/2567.
//

#ifndef KATHRYN_CORE_H
#define KATHRYN_CORE_H

#include "kathryn.h"
#include "example/riscv/parameter.h"
#include "fetch.h"
#include "decode.h"
#include "execute.h"
#include "write_back.h"

namespace kathryn{

    namespace riscv {


        class Riscv : public Module {
        public:

            // m_wire(mis_predic, 1);
            // m_wire(restart_pc, XLEN);
            /** ele*/
            /***bypass ele*/
            CORE_DATA core_data{};

            /** storage*/
            m_mem(reg_file, AMT_REG, XLEN);
            StorageMgmt mem_blk;
            /** pipline element*/
            Fetch       fetch;
            Decode      decode;
            Execute     execute;
            WriteBack   write_back;

            ///FlowBlockPipeWrapper* pip_probe = nullptr;

            explicit Riscv(bool x):
            ///////////// transfer ele
            mem_blk (MEM_ADDR_IDX_ACTUAL_AL32, XLEN), //// -2 due to it is 4 byte align
            ///////////// data path
            fetch  (core_data, mem_blk),
            decode (core_data),
            execute(core_data, mem_blk),
            write_back(core_data){}


            void flow() override {

                /** calulate next cycle*/
                ///// if mispredict occure the execution will write it back
                // cwhile(true){
                //     core_data.pc <<= core_data.pc + 4;
                // }

                /** pipe line wrapper */
                fetch    .flow();
                decode   .flow();
                execute  .flow(reg_file);
                write_back.flow(reg_file);

                mem_blk.build_read_flow();
            }

        };

    }

}

#endif //KATHRYN_CORE_H
