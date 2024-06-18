//
// Created by tanawin on 12/6/2024.
//

#include"RISCV_sim_sort.h"

namespace kathryn::riscv{




    void RiscvSimSort::describeDef(){
        /** start auto simEvent*/
        _riscvSimEvent = new RiscvSimEvent(this);
        getSimController()->addEvent(_riscvSimEvent);
    }

    void RiscvSimSort::dumpMem(uint32_t startAddr, uint32_t stopAddr){
        FileWriterBase dumpFile(_prefixFolder + _testTypes[0] + "/tc" + std::to_string(testCaseId) +
        "_at_"+ std::to_string(startAddr) + ".dumpVar");
        for (uint32_t iter = startAddr; iter < stopAddr; iter = iter + 4){
            dumpFile.addData(std::to_string(_core.memBlk._myMem.at(iter/4).getVal()) + "\n");
        }
    }


}