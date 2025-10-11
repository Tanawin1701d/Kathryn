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

        public:
            mWire(readEn, 1);
            mWire(parCheck, 1);
            Operable&    readFin;
            ////FlowBlockBase* fetchBlock = nullptr;

            explicit Fetch(CORE_DATA& coreData, StorageMgmt& memMgmt):
            cd(coreData),
            storageMgmt(memMgmt),
            readFin(storageMgmt.addReader(readEn,coreData.pc(MEM_ADDR_SL)))
            {}


            void flow(){

                readEn = *cd.dc.sync._syncSlaveReady;

                pip(cd.ft.sync){autoStart
                    zyncc(cd.dc.sync, readFin){
                    /** fetch data is shared among fetch and decoder
                     ** we must m sure it is ready to recv
                     * */
                    cd.ft.fetch_instr  <<= storageMgmt.readOutput;
                    cd.ft.fetch_pc     <<= cd.pc;
                    cd.ft.fetch_nextpc <<= cd.pc + 4;
                }
                }
            }
        };
}

#endif //KATHRYN_FETCH_H
