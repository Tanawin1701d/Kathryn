//
// Created by tanawin on 1/1/26.
//

#include "simCtrlComb.h"

#include <utility>

namespace kathryn::o3{


    CombCtrl::CombCtrl(CYCLE                    limitCycle,
                       const std::string&       prefix,
                       std::vector<std::string> testTypes,
                       SimProxyBuildMode        buildMode,
                       SlotWriterBase&          slotWriter,
                       SimState&                state,
                       TopSim&                  topSim,
                       SimCtrlRide&             slaveRide
    ):
    SimCtrlKride(limitCycle,
                 prefix,
                 std::move(testTypes),
                 buildMode,
                 slotWriter,
                 state,
                 topSim),
    _slaveRide(slaveRide)
    {}


    void CombCtrl::doKrideInit(int curTestCaseIdx){

        _vcdWriter-> renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/owave.vcd");
        _flowWriter->renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oprofile.prof");
        _slotWriter. renew(_prefixFolder + _testTypes[_curTestCaseIdx]+ "/oslot_kride.sl");
        //////// set reset wire to 1
        *rstWire = 1;
        //////// cycle before cycle cycle is running
        conNextCycle(1);
        *rstWire = 0;
        resetRegister();
        readAssembly (_prefixFolder + _testTypes[_curTestCaseIdx] + "/asm.out");
        readAssertVal(_prefixFolder + _testTypes[_curTestCaseIdx] + "/ast.out");
        resetDmem();

    }

    void CombCtrl::doKrideCycle(bool recordThisCycle){
        ///////// give the data to
        readMem2Fetch();
        readWriteDataMemDoCmd(); ///// do the dmem command command

        conEndCycle();
        readWriteDataMemGetCmd();
        ///////// record the system
        _state.recruitValue();
        if (recordThisCycle){
            _state.printSlotWindow(_slotWriter);
        }
        _state.recruitNextCycle();
        postCycleAction(); ///// assign value to the print
        _slotWriter.concludeEachCycle();
        //////////////////////////////////
        conNextCycle(1);

    }

    bool CombCtrl::doCompare(){
        std::cout << TC_BLUE <<
                    "[O3 RISC-V CMP] -----> start compare"
                  << TC_DEF << std::endl;
        bool compareValid = _state.compare(_slaveRide._state);
        compareValid &= compareMemOp(_slaveRide);
        if (compareValid){
            std::cout << TC_GREEN << "[O3 RISC-V CMP] compare pass" << TC_DEF << std::endl;
        }else{
            std::cout << TC_GREEN << "[O3 RISC-V CMP] compare failed see slot writer for the reason mismatch" << TC_DEF << std::endl;
        }
        return compareValid;

    }

    void CombCtrl::describeCon(){

        for (; _curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
            std::cout << std::endl
                      << std::endl
                      << std::endl;
            std::cout << TC_BLUE << "[O3 RISC-V CMP] test type is "
                      << _testTypes[_curTestCaseIdx]
                      << TC_DEF << std::endl;

            ////// init kride and ride
            doKrideInit(_curTestCaseIdx);
            doKrideCycle(false);
            _slaveRide.doRideInit(_curTestCaseIdx);
            //////// iterate for 100 cycle
            bool retard = false;
            int  retartedCount = 0;
            for (int i = 0; i <= 150; i++){
                if (retard && (retartedCount < BELAYED_AFTER_MIS_CMP)){
                    break;
                }
                doKrideCycle(true);
                _slaveRide.doRideCycle(true);

                if (!retard){
                    retard = ~doCompare(); ///// if belayed  = commpare not corect!
                }
                if (retard){
                    if (retartedCount >= BELAYED_AFTER_MIS_CMP){break;}
                    retartedCount++;
                }
            }
            /////////////////////////////////
            testRegister();
            _slaveRide.testRegister();
            finalPerfCol();
        }

    }


}