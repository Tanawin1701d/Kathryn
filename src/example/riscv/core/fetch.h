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

            mWire(readEn, 1);
            StorageMgmt& storageMgmt;
            Reg&    _reqPc;
            Operable&    readFin;
            FlowBlockBase* fetchBlock = nullptr;

            explicit Fetch(StorageMgmt& memMgmt, Reg& reqPc):
            storageMgmt(memMgmt),
            _reqPc(reqPc),
            readFin(storageMgmt.addReader(readEn,_reqPc(MEM_ADDR_SL)))
            {}


            void flow(Operable& rst, FETCH_DATA& fetchdata){

                pipBlk{ intrReset(rst); intrStart(rst); exposeBlk(fetchBlock)
                    par {
                        cdowhile(!readFin){
                            readEn = nextPipReadySig;
                            zif(readFin) {
                                /** fetch data is shared among fetch and decoder
                                 ** we must m sure it is ready to recv
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
