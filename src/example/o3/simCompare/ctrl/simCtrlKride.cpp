//
// Created by tanawin on 25/12/25.
//

#include "simCtrlKride.h"

#include "utility"


namespace kathryn::o3{


    SimCtrlKride::SimCtrlKride(CYCLE                    limitCycle,
                               const std::string&       prefix,
                               std::vector<std::string> testTypes,
                               SimProxyBuildMode        buildMode,
                               SlotWriterBase&          slotWriter,
                               SimState&                state,
                               TopSim&                  topSim,
                               ResultWriter*            resultWriter):

    O3SimCtrlBase(  limitCycle,
                    prefix,
                    std::move(testTypes),
                    buildMode,
                    slotWriter,
                    state,
                    resultWriter),
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
        _slotWriter. renew(_prefixFolder + _testTypes[curTestCaseIdx]+ "/oslotKride.sl");
        doWorkloadExit();
        if (_resultWriter != nullptr){
            _resultWriter->renew(_prefixFolder + _testTypes[curTestCaseIdx]+ "/kathrynKrideResult");
        }
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
            writeMemOp();

            _slotWriter.addSlotVal(RPS_RSV, "--BR-------");
            _slotWriter.addSlotVal(RPS_RSV, "allocPtr " + std::toString(ull(_core.rsvs.br.allocPtr)));
            _slotWriter.addSlotVal(RPS_RSV, "b1"  +   std::toString(ull( _core.rsvs.br.dbgB1Valid))  +   "  value " + std::toString(ull( _core.rsvs.br.dbgB1Idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "e1"  +   std::toString(ull( _core.rsvs.br.dbgE1Valid))  +   "  value " + std::toString(ull( _core.rsvs.br.dbgE1Idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "e0"  +   std::toString(ull( _core.rsvs.br.dbgE0Valid))  +   "  value " + std::toString(ull( _core.rsvs.br.dbgE0Idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "nb1"   + std::toString(ull(  _core.rsvs.br.dbgNb1Valid))  +   "  value " + std::toString(ull(  _core.rsvs.br.dbgNb1Idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "ne1"   + std::toString(ull(  _core.rsvs.br.dbgNe1Valid))  +   "  value " + std::toString(ull(  _core.rsvs.br.dbgNe1Idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "nb0"   + std::toString(ull(  _core.rsvs.br.dbgNb0Valid))  +   "  value " + std::toString(ull(  _core.rsvs.br.dbgNb0Idx))) ;

            _slotWriter.addSlotVal(RPS_RSV, "--LS-------");
            _slotWriter.addSlotVal(RPS_RSV, "allocPtr " + std::toString(ull(_core.rsvs.ls.allocPtr)));
            _slotWriter.addSlotVal(RPS_RSV, "b1"  +   std::toString(ull( _core.rsvs.ls.dbgB1Valid))  +   "  value " + std::toString(ull( _core.rsvs.ls.dbgB1Idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "e1"  +   std::toString(ull( _core.rsvs.ls.dbgE1Valid))  +   "  value " + std::toString(ull( _core.rsvs.ls.dbgE1Idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "e0"  +   std::toString(ull( _core.rsvs.ls.dbgE0Valid))  +   "  value " + std::toString(ull( _core.rsvs.ls.dbgE0Idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "nb1"   + std::toString(ull(  _core.rsvs.ls.dbgNb1Valid))  +   "  value " + std::toString(ull(  _core.rsvs.ls.dbgNb1Idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "ne1"   + std::toString(ull(  _core.rsvs.ls.dbgNe1Valid))  +   "  value " + std::toString(ull(  _core.rsvs.ls.dbgNe1Idx))) ;
            _slotWriter.addSlotVal(RPS_RSV, "nb0"   + std::toString(ull(  _core.rsvs.ls.dbgNb0Valid))  +   "  value " + std::toString(ull(  _core.rsvs.ls.dbgNb0Idx))) ;

            _slotWriter.addSlotVal(RPS_DISPATCH, "--imm1------- "  + cvtNum2HexStr(ull(_core.pDisp.dbgImm1)));
            _slotWriter.addSlotVal(RPS_DISPATCH, "--imm2------- "  + cvtNum2HexStr(ull(_core.pDisp.dbgImm2)));
            _slotWriter.addSlotVal(RPS_DISPATCH, "--inst1------- " + cvtNum2HexStr(ull(_core.pm.dc.dcd1(inst))));
            _slotWriter.addSlotVal(RPS_DISPATCH, "--inst2------- " + cvtNum2HexStr(ull(_core.pm.dc.dcd2(inst))));

            _slotWriter.addSlotVal(RPS_MPFT, "-- cycle --");
            _slotWriter.addSlotVal(RPS_MPFT, "cc: " + std::toString(cycleCnt));

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
        ull dmemWe     = ull(_core.pm.ldSt.dmemWe);
        ull dmemRwaddr = ull(_core.pm.ldSt.dmemRwaddr);
        ull dmemWdata  = ull(_core.pm.ldSt.dmemWdata);
        assert((dmemRwaddr & 0b11) == 0b00);

        lastDmemRead = (dmemWe == 0);
        lastDmemAddr = staticCast<uint32_t>(dmemRwaddr);
        lastDmemWData = staticCast<uint32_t>(dmemWdata);
    }


    void  SimCtrlKride::readWriteDataMemDoCmd (){
        if (!lastDmemEnable){return;}

        ///// At now, lastDmemAddr is quiet sure that there is not polute bit
        uint32_t alignedAddr = lastDmemAddr >> 2;

        if (lastDmemRead){

            if (alignedAddr >= DMEM_ROW){
                std::cout << "skip read due to exceed memory address" << std::endl;
            }else{
                _topSim.ijDmem0.s(_dmem[alignedAddr]);
            }
        }else{

            if (alignedAddr >= DMEM_ROW){
                std::cout << "skip write due to exceed memory address" << std::endl;
            }else{
                _dmem[alignedAddr] = lastDmemWData;
                if ((_resultWriter != nullptr) && (alignedAddr == 0x0)){
                    _resultWriter->fillResult(lastDmemWData);
                }
                if (lastDmemAddr == 0x0 || lastDmemAddr == 0x4 || lastDmemAddr == 0x8){
                    std::cout << "write Detect at KRide @ " << cvtNum2HexStr(lastDmemAddr) << " with data " << lastDmemWData << std::endl;
                }
                //
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
                testAndPrint("fail reg" + std::toString(i),
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


