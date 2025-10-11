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
            mWire(misPredic, 1);
            mWire(restartPc, XLEN);
            /** ele*/
            /***bypass ele*/
            FETCH_DATA  fetchData;
            DECODE_DATA decData;
            EXEC_DATA   execData;
            WRITE_BACK_DATA wbData;
            BYPASS_DATA bp;     ///// bypass data

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
            decData(fetchData.fetch_instr),
            memBlk (MEM_ADDR_IDX_ACTUAL_AL32, XLEN), //// -2 due to it is 4 byte align
            ///////////// data path
            fetch  (memBlk, pc),
            decode(decData),
            execute(decData, memBlk, execData){
                pc.makeResetEvent();
            }


            void flow() override {

                /** calulate next cycle*/
                cwhile(true){
                    zif(misPredic){
                        pc <<= restartPc;
                    }zelif(fetch.readFin){
                        pc <<= pc + 4;
                    }
                }

                /** pipe line wrapper */
                fetch    .flow(misPredic, fetchData, decData);
                decode   .flow(misPredic, fetchData, execData);
                execute  .flow(misPredic, restartPc, regFile,
                               bp, execData, wbData);
                writeBack.flow(execData.wbData, regFile,
                               execData, bp);



                memBlk.buildReadFlow();
            }

        };

    }

}

#endif //KATHRYN_CORE_H
