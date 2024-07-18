//
// Created by tanawin on 15/6/2024.
//

#include "autoSimEvent.h"
#include "RISCV_sim_sort.h"

namespace kathryn::riscv{

    RiscvSimEvent::RiscvSimEvent(RiscvSimSort* master):
    EventBase(2, SIM_CC_TRIGGER_PRIO_FRONT_CYCLE),
    _master(master)
    {
        assert(_master != nullptr);
    }

    void RiscvSimEvent::simStartCurCycle(){


        /**
         *
         * initialize data
         *
         */
        std::cout << TC_GREEN << "testing riscv instruction SORT >>>> " +
    std::to_string(_master->calculateAmtNumber()) << TC_DEF << std::endl;
        *rstWire = 1;
        _master->getProxySimEvent()->simStartCurCycle();
        _master->getProxySimEvent()->curCycleCollectData();
        _master->getProxySimEvent()->simStartNextCycle();
        _master->getProxySimEvent()->simExitCurCycle();

        *rstWire = 0;
        _master->_core.pc = 0;
        _master->resetRegister();
        int testSizeId = _master->testCaseId;
        std::string testSizeStr = std::to_string(testSizeId);
        std::cout << TC_GREEN << "initializing mem .... >>>> " << TC_DEF << std::endl;
        _master->readAssembly(_master->_prefixFolder +  _master->_testTypes[0] +
                              "/asm" + testSizeStr + ".out");
        /** generate value to system*/
        for (int i = 0; i < _master->calculateAmtNumber(); i++){
            _master->_core.memBlk._myMem.at((_master->_startNumIdx0 + 4*i)/4)
            .setVar(_master->calculateAmtNumber() - i + 10);
        }
        //////////////////////////////////////////////////////////////////////////////
        _master->readAssertVal(_master->_prefixFolder + _master->_testTypes[0] + "/ast.out");

        _master->getProxySimEvent()->simStartCurCycle();
        _master->getProxySimEvent()->curCycleCollectData();
        _master->getProxySimEvent()->simStartNextCycle();
        _master->getProxySimEvent()->simExitCurCycle();

        //*
        // simulation master
        //
        //*/
        auto start = std::chrono::steady_clock::now();

        ull& x = _master->_core.regFile.at(testFinRegIdx).getRefVal();
        ProxySimEventBase* proxy = _master->getProxySimEvent();
        while(x != 1){
            // if ( (_curCycleCal % (((ull)1) << 20)) == 0){
            //     std::cout << "now simulate pass " << _curCycleCal << std::endl;
            // }
            proxy->simStartCurCycle();
            proxy->curCycleCollectData();
            /** slot recorder*/

            //////
            proxy->simStartNextCycle();
            proxy->simExitCurCycle();
            _curCycleCal++;
        }

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "process time time: " << elapsed_seconds.count() << "s\n";

        std::cout << "end at cycle " << std::to_string(_curCycleCal) << std::endl;
        std::cout << "dummping mem 0" << std::endl;
        _master->dumpMem(_master->_startNumIdx0, _master->_startNumIdx0 + (_master->calculateAmtNumber() * 4));
        std::cout << "dummping mem 1" << std::endl;
        _master->dumpMem(_master->_startNumIdx1, _master->_startNumIdx1 + (_master->calculateAmtNumber() * 4));
        std::cout << "dump finish" << std::endl;

    }

    EventBase* RiscvSimEvent::genNextEvent(){
        _targetCycle += _curCycleCal + 100;
        _curCycleCal = 0;
        _master->testCaseId++;
        if (_master->testCaseId < _master->AMT_TEST_CASE){
            return this;
        }
        return nullptr;
    }


}