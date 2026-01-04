//
// Created by tanawin on 25/12/25.
//

#include "simCtrlKride.h"

#include <utility>


namespace kathryn::o3{


    SimCtrlKride::SimCtrlKride(CYCLE                    limitCycle,
                               const std::string&       prefix,
                               std::vector<std::string> testTypes,
                               SimProxyBuildMode        buildMode,
                               SlotWriterBase&          slotWriter,
                               SimState&                state,
                               TopSim&                  topSim):

    O3SimCtrlBase(  limitCycle,
                    prefix,
                    std::move(testTypes),
                    buildMode,
                    slotWriter,
                    state),
    SimInterface(limitCycle,
                 "/tmp/vcdDummy",
                 "/tmp/prodummy",
                 "O3RiscV",
                 buildMode,
                 false,
                 false,
                 1),
    _topSim(topSim),
    _core(topSim.myCore){}

    void SimCtrlKride::describeCon(){

        for (; _curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
            std::cout << TC_BLUE << "[O3 RISC-V] test type is " << _testTypes[_curTestCaseIdx] << TC_DEF << std::endl;
            //////////////  read assembly and assertVal

            _vcdWriter-> renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/owave.vcd");
            _flowWriter->renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oprofile.prof");
            _slotWriter. renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oslot.sl");
            //////// set reset wire to 1
            *rstWire = 1;
            //////// cycle before cycle cycle is running
            conNextCycle(1);
            *rstWire = 0;
            resetRegister();
            readAssembly (_prefixFolder + _testTypes[_curTestCaseIdx] + "/asm.out");
            readAssertVal(_prefixFolder + _testTypes[_curTestCaseIdx] + "/ast.out");
            resetDmem();
            //////// iterate for 100 cycle
            for (int i = 0; i <= 150; i++){
                ///////// give the data to
                readMem2Fetch();
                readWriteDataMemDoCmd(); ///// do the dmem command command

                conEndCycle();
                readWriteDataMemGetCmd();
                ///////// record the system
                _state.recruitValue();
                _state.printSlotWindow(_slotWriter);
                _state.recruitNextCycle();
                postCycleAction(); ///// assign value to the print
                _slotWriter.concludeEachCycle();
                //////////////////////////////////
                conNextCycle(1);
            }
            /////////////////////////////////
            testRegister();
            finalPerfCol();
        }
    }

    void  SimCtrlKride::readMem2Fetch(){

        ///// get new instruction data
        ull curPc     = ull(_core.pm.ft.curPc);
        curPc         = curPc >> 2; ///// make 4bytes align
        ull aligner   = (ull(1) << 2) - 1; ///// to align 4 instructions per read 111111...11100
        aligner       = (~aligner);
        ull alignedPc = curPc & aligner;

        ///// get new instruction data
        _topSim.ijImem0.s(_imem[alignedPc + 0]);
        _topSim.ijImem1.s(_imem[alignedPc + 1]);
        _topSim.ijImem2.s(_imem[alignedPc + 2]);
        _topSim.ijImem3.s(_imem[alignedPc + 3]);

    }

    void  SimCtrlKride::readWriteDataMemGetCmd(){
        ///// make command enable
        lastDmemEnable = true;
        ///// read data from CPU
        ull dmem_we     = ull(_core.pm.ldSt.dmem_we);
        ull dmem_rwaddr = ull(_core.pm.ldSt.dmem_rwaddr);
        ull dmem_wdata  = ull(_core.pm.ldSt.dmem_wdata);
        assert((dmem_rwaddr & 0b11) == 0b00);

        lastDmemRead = (dmem_we == 0);
        lastDmemAddr = static_cast<uint32_t>(dmem_rwaddr);
        lastDmemWData = static_cast<uint32_t>(dmem_wdata);
    }


    void  SimCtrlKride::readWriteDataMemDoCmd (){
        if (!lastDmemEnable){return;}

        ///// At now, lastDmemAddr is quiet sure that there is not polute bit
        uint32_t aligned_addr = lastDmemAddr >> 2;

        if (lastDmemRead){
            _topSim.ijDmem0.s(_dmem[aligned_addr]);
        }else{
            _dmem[aligned_addr] = lastDmemWData;
            std::cout << "write Detect at @ " << cvtNum2HexStr(lastDmemAddr) << " with data " << lastDmemWData << std::endl;
        }
    }
    void  SimCtrlKride::resetRegister(){
        for (int i = 0; i < REG_NUM; i++){
            _core.regArch.arf.archRegs(i) = 0;
        }
    }
    void  SimCtrlKride::testRegister(){
        bool pass = true;
        for (int i = 0;  i < REG_NUM; i++){
            if (_regTestVal[i] != (ull)_core.regArch.arf.archRegs(i)){
                pass = false;
                testAndPrint("fail reg" + std::to_string(i),
                             (ull)_core.regArch.arf.archRegs(i), _regTestVal[i]);
            }
        }
        if (pass){
            std::cout << TC_GREEN << "register val test pass" << TC_DEF << std::endl;
        }else{
            std::cout << TC_RED << "register val test fail" << TC_DEF << std::endl;
        }
    }

    void SimCtrlKride::postCycleAction(){



    }

}


