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
            CORE_DATA& cd;
            StorageMgmt& storageMgmt;
            Reg&    _reqPc;

        public:
            mWire(readEn, 1);
            mWire(parCheck, 1);
            Operable&    readFin;
            ////FlowBlockBase* fetchBlock = nullptr;

            explicit Fetch(CORE_DATA& coreData, StorageMgmt& memMgmt, Reg& reqPc):
            cd(coreData),
            storageMgmt(memMgmt),
            _reqPc(reqPc),
            readFin(storageMgmt.addReader(readEn,_reqPc(MEM_ADDR_SL)))
            {}


            void flow(){

                readEn = *cd.dc.sync._syncSlaveReady;

                pip(cd.ft.sync){autoStart
                    zyncc(cd.dc.sync, readFin){
                    /** fetch data is shared among fetch and decoder
                     ** we must m sure it is ready to recv
                     * */
                    cd.ft.fetch_instr  <<= storageMgmt.readOutput;
                    cd.ft.fetch_pc     <<= _reqPc;
                    cd.ft.fetch_nextpc <<= _reqPc + 4;
                }
                }
            }
        };
}

#endif //KATHRYN_FETCH_H
