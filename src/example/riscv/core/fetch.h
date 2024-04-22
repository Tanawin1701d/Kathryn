//
// Created by tanawin on 3/4/2567.
//

#ifndef KATHRYN_FETCH_H
#define KATHRYN_FETCH_H

#include "kathryn.h"
#include "example/riscv/subSystem/storageMgm.h"
#include "example/riscv/parameter.h"
#include "example/riscv/element.h"

namespace kathryn{

    namespace riscv {

        class Fetch {


        public:
            /** to send to next instruction*/

            //makeReg(valid, 1);

            makeWire(readEn, 1);
            Operable* readFin = nullptr;


            void flow(Operable& rst, Operable& reqPc, StorageMgmt& memMgmt, FETCH_DATA& fetchdata) {


                readFin = &memMgmt.addReader(readEn,reqPc);

                pipBlk {
                    intrReset(rst);
                    cwhile(true) {
                        par {
                            readEn        = 1;
                            fetchdata.fetch_instr <<= memMgmt.readOutput;
                            zif(*readFin){
                                fetchdata.fetch_pc     <<= reqPc;
                                fetchdata.fetch_nextpc <<= reqPc + 4;
                            }
                            sbreakCon(*readFin);
                        }
                    }
                }
            }

        };

    }

}

#endif //KATHRYN_FETCH_H
