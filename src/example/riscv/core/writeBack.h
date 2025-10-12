//
// Created by tanawin on 8/4/2567.
//

#ifndef KATHRYN_WRITEBACK_H
#define KATHRYN_WRITEBACK_H

#include "kathryn.h"
#include "example/riscv/element.h"
#include "example/riscv/subSystem/storageMgm.h"

namespace kathryn::riscv{

        struct WriteBack{
            CORE_DATA& cd;
            PipSimProbe pipSimProbe;
        public:

            WriteBack(CORE_DATA& coreData): cd(coreData){}

            void flow(MemBlock& regFile){

                OPR_HW& desReg = cd.ex.wbData;
                desReg.idx  .asOutputGlob("writeIdx");
                desReg.data .asOutputGlob("writeData");
                desReg.valid.asOutputGlob("valid");


                pip(cd.wb.sync){ initProbe(pipSimProbe);
                    zif((desReg.valid) && (desReg.idx != 0)) {
                        regFile[desReg.idx] <<= desReg.data;
                        cd.bp.idx = desReg.idx;
                        cd.bp.value = desReg.data;
                    }
                }
            }
        };
}

#endif //KATHRYN_WRITEBACK_H
