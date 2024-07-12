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
            UOp         decData;
            RegEle      wbData; //// write back data
            BYPASS_DATA bp;     ///// bypass data

            /** storage*/
            mMem(regFile, AMT_REG, XLEN);
            StorageMgmt memBlk;
            /** pipline element*/
            Fetch       fetch;
            Decode      decode;
            Execute     execute;
            WriteBack   writeBack;

            FlowBlockPipeWrapper* pipProbe = nullptr;

            explicit Riscv(bool x):
            memBlk (MEM_ADDR_IDX_ACTUAL_AL32, XLEN), //// -2 due to it is 4 byte align
            fetch  (memBlk, pc),
            execute(decData, memBlk, wbData){}

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
                pipWrap{ exposeBlk(pipProbe)
                    fetch    .flow(misPredic, fetchData);
                    decode   .flow(misPredic, fetchData, decData);
                    /**execute and write back can't be delete anymore*/
                    execute  .flow(misPredic, restartPc, regFile, bp); ///////// mispredict writer
                    writeBack.flow(wbData, regFile, bp);
                }

                memBlk.buildReadFlow();
            }

        };

    }

}

#endif //KATHRYN_CORE_H
