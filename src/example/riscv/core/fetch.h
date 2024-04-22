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
            StorageMgmt& storageMgmt;
            Operable&    _reqPc;
            Operable&    readFin;

            explicit Fetch(StorageMgmt& memMgmt, Operable& reqPc):
            storageMgmt(memMgmt),
            _reqPc(reqPc),
            readFin(storageMgmt.addReader(readEn,_reqPc))
            {}


            void flow(Operable& rst, FETCH_DATA& fetchdata) {

                pipBlk {
                    intrReset(rst);
                    par {
                        cdowhile(!readFin){
                            readEn = 1;
                            fetchdata.fetch_instr <<= storageMgmt.readOutput;
                            zif(readFin) {
                                fetchdata.fetch_pc <<= _reqPc;
                                fetchdata.fetch_nextpc <<= _reqPc + 4;
                            }
                        }
                    }
                }
            }

        };

    }

}

#endif //KATHRYN_FETCH_H
