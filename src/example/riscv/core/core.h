//
// Created by tanawin on 1/4/2567.
//

#ifndef KATHRYN_CORE_H
#define KATHRYN_CORE_H

#include"kathryn.h"
#include"example/riscv/parameter.h"
#include"fetch.h"
#include"decode.h"
#include"execute.h"
#include"writeBack.h"

namespace kathryn{

    namespace riscv {


        class Riscv : public Module {
        public:

            mReg (pc       , XLEN);
            // mWire(misPredic, 1);
            // mWire(restartPc, XLEN);
            /** ele*/
            /***bypass ele*/
            CORE_DATA coreData{};

            /** storage*/
            mMem(regFile, AMT_REG, XLEN);
            StorageMgmt memBlk;
            /** pipline element*/
            Fetch       fetch;
            Decode      decode;
            Execute     execute;
            WriteBack   writeBack;

            ///FlowBlockPipeWrapper* pipProbe = nullptr;

            explicit Riscv(bool x):
            ///////////// transfer ele
            memBlk (MEM_ADDR_IDX_ACTUAL_AL32, XLEN), //// -2 due to it is 4 byte align
            ///////////// data path
            fetch  (coreData, memBlk, pc),
            decode (coreData),
            execute(coreData, memBlk){
                pc.makeResetEvent();
            }


            void flow() override {

                /** calulate next cycle*/
                ///// if mispredict occure the execution will write it back
                cwhile(true){
                    pc <<= pc + 4;
                }

                /** pipe line wrapper */
                fetch    .flow();
                decode   .flow();
                execute  .flow(regFile);
                writeBack.flow(regFile);

                memBlk.buildReadFlow();
            }

        };

    }

}

#endif //KATHRYN_CORE_H
