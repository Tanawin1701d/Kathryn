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

            makeReg(pc, XLEN);
            makeWire(misPredic, 1);
            makeWire(restartPc, XLEN);
            /** storage*/
            makeMem(regFile, AMT_REG, XLEN);
            StorageMgmt memBlk;
            /** pipline element*/
            Fetch     fetch;
            Decode    decode;
            Execute   execute;
            WriteBack writeBack;
            /***bypass ele*/
            FETCH_DATA  fetchData;
            BYPASS_DATA bp;

            explicit Riscv():

            memBlk(MEM_ADDR_IDX-2, XLEN),
            fetch(memBlk, pc),
            execute(decode.decInstr, memBlk){}

            void flow() override {

                cwhile(true){
                    zif(misPredic){
                        pc <<= restartPc;
                    }zelif(fetch.readFin){
                        pc <<= pc + 4;
                    }
                }

                pipWrap{
                    fetch    .flow(misPredic, fetchData);
                    decode   .flow(misPredic, fetchData);
                    /**execute and write back can't be delete anymore*/
                    execute  .flow(misPredic, restartPc, regFile, bp); ///////// mispredict writer
                    writeBack.flow(execute.exUop, regFile,bp);
                }
            }

        };

    }

}

#endif //KATHRYN_CORE_H
