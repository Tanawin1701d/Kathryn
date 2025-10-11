//
// Created by tanawin on 3/4/2567.
//

#ifndef KATHRYN_FETCH_H
#define KATHRYN_FETCH_H

#include "kathryn.h"
#include "example/riscv/subSystem/storageMgm.h"
#include "example/riscv/parameter.h"
#include "example/riscv/element.h"

namespace kathryn::riscv {

        class Fetch {


        public:

            mWire(readEn, 1);
            mWire(parCheck, 1);
            StorageMgmt& storageMgmt;
            Reg&    _reqPc;
            Operable&    readFin;
            ////FlowBlockBase* fetchBlock = nullptr;

            explicit Fetch(StorageMgmt& memMgmt, Reg& reqPc):
            storageMgmt(memMgmt),
            _reqPc(reqPc),
            readFin(storageMgmt.addReader(readEn,_reqPc(MEM_ADDR_SL)))
            {}


            void flow(Operable& rst, FETCH_DATA& fetData, DECODE_DATA& decData){

                readEn = *decData.sync._syncSlaveReady;

                pip(fetData.sync){autoStart
                    zyncc(decData.sync, readFin){
                    /** fetch data is shared among fetch and decoder
                     ** we must m sure it is ready to recv
                     * */
                    fetData.fetch_instr <<= storageMgmt.readOutput;
                    fetData.fetch_pc     <<= _reqPc;
                    fetData.fetch_nextpc <<= _reqPc + 4;
                }
                }
            }
        };
}

#endif //KATHRYN_FETCH_H
