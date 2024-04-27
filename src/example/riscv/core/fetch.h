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

            makeWire(readEn, 1);
            StorageMgmt& storageMgmt;
            Operable&    _reqPc;
            Operable&    readFin;

            explicit Fetch(StorageMgmt& memMgmt, Operable& reqPc):
            storageMgmt(memMgmt),
            _reqPc(reqPc),
            readFin(storageMgmt.addReader(readEn,_reqPc))
            {}


            void flow(Operable& rst, FETCH_DATA& fetchdata){

                pipBlk{ intrReset(rst); intrStart(rst);
                    par {
                        cdowhile(!readFin){
                            readEn = nextPipReadySig;
                            zif(readFin) {
                                /** fetch data is shared among fetch and decoder
                                 ** we must make sure it is ready to recv
                                 * */
                                fetchdata.fetch_instr <<= storageMgmt.readOutput;
                                fetchdata.fetch_pc     <<= _reqPc;
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
