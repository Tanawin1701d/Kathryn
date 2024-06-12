//
// Created by tanawin on 12/6/2024.
//

#include"RISCV_sim_sort.h"

namespace kathryn::riscv{


    void RiscvSimSort::writeMem(){
        for (int i = 0; i < 4; i++){
            slotWriter.addSlotVal(RISC_MEM, "idx" + std::to_string(i) + " " +
                                    std::to_string(ull(_core.memBlk._myMem.at((_startNumIdx0 + 4*i)/4))) + " addr " + std::to_string((_startNumIdx0 + 4*i)/4));
        }
        slotWriter.addSlotVal(RISC_MEM, "------------------------------");
        for (int i = 0; i < 4; i++){
            slotWriter.addSlotVal(RISC_MEM, "idx" + std::to_string(i) + " " +
                                            std::to_string((ull)(_core.memBlk._myMem.at((_startNumIdx1 + 4*i)/4 )))
                                            + " addr " + std::to_string((_startNumIdx1 + 4*i)/4)
                                            );
        }

    }

    void RiscvSimSort::describeCon(){

        ull curCycleChecker = 0;
        ull conLimitCycle = _limitCycle - 10;
        //////////////  read assembly and assertVal
        std::cout << TC_GREEN << "testing riscv instruction >>>> " +
            _testTypes[_curTestCaseIdx] << TC_DEF << std::endl;
        _vcdWriter-> renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/owave.vcd");
        _flowWriter->renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oprofile.prof");
        slotWriter.  renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oslot.sl");

        /////////// set reset wire to 1
        *rstWire = 1;
        ////////// value before cycle is running
        conNextCycle(1);
        *rstWire = 0;
        _core.pc = 0;
        resetRegister();
        readAssembly(_prefixFolder +  _testTypes[_curTestCaseIdx] + "/asm.out");
        readAssertVal(_prefixFolder + _testTypes[_curTestCaseIdx] + "/ast.out");
        //////////////////////////////////////////////////////////////////////
        auto start = std::chrono::steady_clock::now();
        while ( (_core.regFile.at(testFinRegIdx).getVal() !=  1) &&
            (curCycleChecker < conLimitCycle)){
            if ( (curCycleChecker % (((ull)1) << 20)) == 0){
                std::cout << "pass " << curCycleChecker  << std::endl;
            }
            conEndCycle();
            //recordSlot();
            conNextCycle(1);
            curCycleChecker++;
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "process time time: " << elapsed_seconds.count() << "s\n";
        //////////////////////////////////////////////////////////////////////
        testRegister();
        ////////////////////////////////////////////////////////////////////////
        if (curCycleChecker >= conLimitCycle){
            std::cout << TC_RED << "system hault due to exceed limit cycle" << TC_DEF << std::endl;
        }else{
            std::cout << TC_GREEN << "system hault due to finish execution" << TC_DEF << std::endl;
        }
        std::cout << "end at cycle " << std::to_string(curCycleChecker) << std::endl;
        std::cout << "dummping mem 0" << std::endl;
        dumpMem(_startNumIdx0, _startNumIdx0 + (_numSize * 4));
        std::cout << "dummping mem 1" << std::endl;
        dumpMem(_startNumIdx1, _startNumIdx1 + (_numSize * 4));
        std::cout << "dump finish" << std::endl;
    }


    void RiscvSimSort::readAssembly(const std::string& filePath){

        RiscvSim::readAssembly(filePath);

        for (int i = 0; i < _numSize; i++){
            _core.memBlk._myMem.at((_startNumIdx0 + 4*i)/4).setVar(_numSize - i + 10);
        }
    }

    void RiscvSimSort::dumpMem(uint32_t startAddr, uint32_t stopAddr){
        FileWriterBase dumpFile(_prefixFolder + _testTypes[0] + "/" + std::to_string(startAddr) + ".dumpVar");
        for (uint32_t iter = startAddr; iter < stopAddr; iter = iter + 4){
            dumpFile.addData(std::to_string(_core.memBlk._myMem.at(iter/4).getVal()) + "\n");
        }
    }


}