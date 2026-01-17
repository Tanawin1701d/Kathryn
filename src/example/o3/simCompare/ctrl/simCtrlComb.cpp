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
                       SimCtrlRide&             slaveRide,
                       bool                     reqRegTest,
                       ResultWriter*            resultWriter
    ):
    SimCtrlKride(limitCycle,
                 prefix,
                 std::move(testTypes),
                 buildMode,
                 slotWriter,
                 state,
                 topSim,
                 resultWriter),
    _slaveRide  (slaveRide),
    _reqRegTest (reqRegTest)
    {}

    bool CombCtrl::doCompare(){
        bool compareValid = _state.compare(_slaveRide._state);
        compareValid &= compareMemOp(_slaveRide);
        return compareValid;

    }

    void CombCtrl::describeCon(){

        std::vector<int> errorIndexs;

        for (; _curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
            std::cout << std::endl
                      << std::endl
                      << std::endl;
            std::cout << TC_BLUE << "[O3 RISC-V CMP] test type is "
                      << _testTypes[_curTestCaseIdx]
                      << TC_DEF << std::endl;

            ////// init kride and ride
            doWorkloadInit(_curTestCaseIdx, _reqRegTest);
            doWorkloadCycle(false);
            _slaveRide.doWorkloadInit(_curTestCaseIdx, _reqRegTest);
            //////// iterate for 100 cycle
            bool retard = false;
            int  retartedCount = 0;
            std::cout << TC_BLUE <<
                    "[O3 RISC-V CMP] -----> start compare"
                  << TC_DEF << std::endl;
            while (true){
                if (retard && (retartedCount < BELAYED_AFTER_MIS_CMP)){
                    break;
                }
                doWorkloadCycle(true);
                _slaveRide.doWorkloadCycle(true);

                if (!retard){
                    retard = !doCompare(); ///// if belayed  = commpare not corect!
                }
                if (retard){
                    if (retartedCount >= BELAYED_AFTER_MIS_CMP){break;}
                    retartedCount++;
                }


                if (isExecFin() && _slaveRide.isExecFin()){
                    std::cout << TC_GREEN << "slave is equal " << TC_DEF << std::endl;
                    break;
                }else if (isExecFin()){
                    std::cout << TC_RED << "master is finish but slave not" << TC_DEF << std::endl;
                    break;
                }else if (_slaveRide.isExecFin()){
                    std::cout << TC_RED << "slave is  finish not like" << TC_DEF << std::endl;
                    break;
                }
                if (cycleCnt % 10000 == 0){
                    std::cout << TC_BLUE << "[O3 RISC-V CMP] -----> computing cycle " << cycleCnt << TC_DEF << std::endl;
                }

                ////// increase cycle counter
                incCycleCnt();
                _slaveRide.incCycleCnt();
            }
            std::cout << TC_BLUE << "[O3 RISC-V CMP] -----> sim done in " << cycleCnt << " cycles" << TC_DEF << std::endl;

            if (retard){
                std::cout << TC_RED << "[O3 RISC-V CMP] compare failed see slot writer for the reason mismatch" << TC_DEF << std::endl;
                errorIndexs.push_back(_curTestCaseIdx);
            }else{
                std::cout << TC_GREEN << "[O3 RISC-V CMP] compare pass" << TC_DEF << std::endl;
            }
            /////////////////////////////////

            if (_reqRegTest){
                testRegister();
                _slaveRide.testRegister();
            }
            finalPerfCol();
        }

        for (int errorIdx: errorIndexs){
            std::cout << TC_RED << "[O3 RISC-V CMP] error in test case " << errorIdx << TC_DEF << std::endl;
        }

    }


}