//
// Created by tanawin on 12/6/2024.
//

#include"RISCV_sim_sort.h"

namespace kathryn::riscv{



    void RiscvSimSort::prepareNextTc(){

        if(testCaseId != -1){
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "end at cycle " << std::to_string(_modelSimEvent->getAmtLRsim()) << std::endl;
            std::cout << "process time time: " << elapsed_seconds.count() << "s\n";
            std::cout << "dummping mem 0" << std::endl;
            dumpMem(_startNumIdx0, _startNumIdx0 + (calculateAmtNumber() * 4));
            std::cout << "dummping mem 1" << std::endl;
            dumpMem(_startNumIdx1, _startNumIdx1 + (calculateAmtNumber() * 4));
            std::cout << "dump finish" << std::endl;

            CYCLE curCycle       = getSimController()->getCurCycle();
            CYCLE longRangeCycle = _modelSimEvent->getAmtLRsim();

            new UserEvent([&](UserEvent&){
                *rstWire = 1;
            }, curCycle + longRangeCycle, SIM_USER_PRIO_FRONT_CYCLE);
            new UserEvent([&](UserEvent&){
                *rstWire = 0;
            }, curCycle + longRangeCycle + 1, SIM_USER_PRIO_FRONT_CYCLE);

            ////////// send reset signal to next cyclc
        }

        finalPerfCol();

        testCaseId++;
        if (testCaseId == AMT_TEST_CASE){
            getSimController()->stopSim();
            return;
        }



        std::string testSizeStr = std::to_string(testCaseId);
        resetRegister();
        readAssembly(_prefixFolder + _testTypes[0] + "/asm" + testSizeStr + ".out");
        _flowWriter->renew(_prefixFolder + _testTypes[0]+ "/oprofile"+ testSizeStr +".prof");
        for (int i = 0; i < calculateAmtNumber(); i++){
            _core.memBlk._myMem.at((_startNumIdx0 + 4*i)/4)
            .setVar(calculateAmtNumber() - i + 10);
        }
        start = std::chrono::steady_clock::now();


    }

    void RiscvSimSort::describeModelTrigger(){
        trig(_core.regFile[31] == 1, [&](){prepareNextTc();});
    }

    void RiscvSimSort::describeDef(){
        prepareNextTc();
        SimInterface::describeDef();
        _modelSimEvent->setLongRunType(true);

    }


    void RiscvSimSort::describeCon(){}



    void RiscvSimSort::dumpMem(uint32_t startAddr, uint32_t stopAddr){
        FileWriterBase dumpFile(_prefixFolder + _testTypes[0] + "/tc" + std::to_string(testCaseId) +
        "_at_"+ std::to_string(startAddr) + ".dumpVar");
        for (uint32_t iter = startAddr; iter < stopAddr; iter = iter + 4){
            dumpFile.addData(std::to_string(_core.memBlk._myMem.at(iter/4).getVal()) + "\n");
        }
    }
}