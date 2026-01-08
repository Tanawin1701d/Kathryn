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

    void SimCtrlKride::doWorkloadInit(int curTestCaseIdx, bool reqRegTest){

        _vcdWriter-> renew(_prefixFolder + _testTypes[curTestCaseIdx]+ "/owave.vcd");
        _flowWriter->renew(_prefixFolder + _testTypes[curTestCaseIdx]+ "/oprofile.prof");
        _slotWriter. renew(_prefixFolder + _testTypes[curTestCaseIdx]+ "/oslot_kride.sl");
        //////// set reset wire to 1
        *rstWire = 1;
        //////// cycle before cycle cycle is running
        conNextCycle(1);
        *rstWire = 0;
        resetRegister();
        readAssembly (_prefixFolder + _testTypes[curTestCaseIdx] + "/asm.out");
        if (reqRegTest){
            readAssertVal(_prefixFolder + _testTypes[curTestCaseIdx] + "/ast.out");
        }
        //resetDmem();   we dont use it any more na krub

    }

    void SimCtrlKride::doWorkloadCycle(bool recordThisCycle){
        ///////// give the data to
        readMem2Fetch();
        readWriteDataMemDoCmd(); ///// do the dmem command command

        conEndCycle();
        readWriteDataMemGetCmd();
        ///////// record the system
        _state.recruitValue();
        if (recordThisCycle){
            _state.printSlotWindow(_slotWriter);

            _slotWriter.addSlotVal(RPS_RSV, "--BR-------");
            _slotWriter.addSlotVal(RPS_RSV, "allocPtr " + std::to_string(ull(_core.rsvs.br.allocPtr)));
            _slotWriter.addSlotVal(RPS_RSV, "b1"  +   std::to_string(ull( _core.rsvs.br.dbg_b1_valid))  +   "  value " + std::to_string(ull( _core.rsvs.br.dbg_b1_idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "e1"  +   std::to_string(ull( _core.rsvs.br.dbg_e1_valid))  +   "  value " + std::to_string(ull( _core.rsvs.br.dbg_e1_idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "e0"  +   std::to_string(ull( _core.rsvs.br.dbg_e0_valid))  +   "  value " + std::to_string(ull( _core.rsvs.br.dbg_e0_idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "nb1"   + std::to_string(ull(  _core.rsvs.br.dbg_nb1_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.br.dbg_nb1_idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "ne1"   + std::to_string(ull(  _core.rsvs.br.dbg_ne1_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.br.dbg_ne1_idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "nb0"   + std::to_string(ull(  _core.rsvs.br.dbg_nb0_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.br.dbg_nb0_idx))) ;

            _slotWriter.addSlotVal(RPS_RSV, "--LS-------");
            _slotWriter.addSlotVal(RPS_RSV, "allocPtr " + std::to_string(ull(_core.rsvs.ls.allocPtr)));
            _slotWriter.addSlotVal(RPS_RSV, "b1"  +   std::to_string(ull( _core.rsvs.ls.dbg_b1_valid))  +   "  value " + std::to_string(ull( _core.rsvs.ls.dbg_b1_idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "e1"  +   std::to_string(ull( _core.rsvs.ls.dbg_e1_valid))  +   "  value " + std::to_string(ull( _core.rsvs.ls.dbg_e1_idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "e0"  +   std::to_string(ull( _core.rsvs.ls.dbg_e0_valid))  +   "  value " + std::to_string(ull( _core.rsvs.ls.dbg_e0_idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "nb1"   + std::to_string(ull(  _core.rsvs.ls.dbg_nb1_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.ls.dbg_nb1_idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "ne1"   + std::to_string(ull(  _core.rsvs.ls.dbg_ne1_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.ls.dbg_ne1_idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "nb0"   + std::to_string(ull(  _core.rsvs.ls.dbg_nb0_valid))  +   "  value " + std::to_string(ull(  _core.rsvs.ls.dbg_nb0_idx))) ;

            _slotWriter.addSlotVal(RPS_DISPATCH, "--imm1------- "  + cvtNum2HexStr(ull(_core.pDisp.dbg_imm1)));
            _slotWriter.addSlotVal(RPS_DISPATCH, "--imm2------- "  + cvtNum2HexStr(ull(_core.pDisp.dbg_imm2)));
            _slotWriter.addSlotVal(RPS_DISPATCH, "--inst1------- " + cvtNum2HexStr(ull(_core.pm.dc.dcd1(inst))));
            _slotWriter.addSlotVal(RPS_DISPATCH, "--inst2------- " + cvtNum2HexStr(ull(_core.pm.dc.dcd2(inst))));



        }
        _state.recruitNextCycle();
        postCycleAction(); ///// assign value to the print
        _slotWriter.concludeEachCycle();
        //////////////////////////////////
        conNextCycle(1);
    }

    void SimCtrlKride::describeCon(){

        for (; _curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
            std::cout << TC_BLUE << "[O3 RISC-V] test type is " << _testTypes[_curTestCaseIdx] << TC_DEF << std::endl;
            //////////////  read assembly and assertVal
            doWorkloadInit(_curTestCaseIdx, true);
            //////// iterate for 100 cycle
            for (int i = 0; i <= 150; i++){
                doWorkloadCycle(true);
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

            if (aligned_addr >= DMEM_ROW){
                std::cout << "skip read due to exceed memory address" << std::endl;
            }else{
                _topSim.ijDmem0.s(_dmem[aligned_addr]);
            }
        }else{

            if (aligned_addr >= DMEM_ROW){
                std::cout << "skip write due to exceed memory address" << std::endl;
            }else{
                _dmem[aligned_addr] = lastDmemWData;
                std::cout << "write Detect at @ " << cvtNum2HexStr(lastDmemAddr) << " with data " << lastDmemWData << std::endl;
            }


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


