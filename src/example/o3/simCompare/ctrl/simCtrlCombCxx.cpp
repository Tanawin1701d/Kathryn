// //
// // Created by tanawin on 6/1/26.
// //
//
// #include "simCtrlCombCxx.h"
//
// namespace kathryn::o3{
//
//
//     CombCtrlCxx::CombCtrlCxx(CYCLE       limitCycle,
//                 const std::string&       prefix,
//                 std::vector<std::string> testTypes,
//                 SimProxyBuildMode        buildMode,
//                 SlotWriterBase&          slotWriter,
//                 SimState&                state,
//                 TopSim&                  topSim,
//                 SimCtrlRide&             slaveRide):
//     CombCtrl(limitCycle,
//              prefix,
//              testTypes,
//              buildMode,
//              slotWriter,
//              state,
//              topSim,
//              slaveRide){}
//
//     void CombCtrlCxx::describeCon(){
//
//         for (; _curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
//             std::cout << std::endl
//                       << std::endl
//                       << std::endl;
//             std::cout << TC_BLUE << "[O3 RISC-V CMP] test type is "
//                       << _testTypes[_curTestCaseIdx]
//                       << TC_DEF << std::endl;
//
//             ////// init kride and ride
//             doWorkloadInit(_curTestCaseIdx, false);
//             doWorkloadCycle(false);
//             _slaveRide.doWorkloadInit(_curTestCaseIdx, false);
//             //////// iterate for 100 cycle
//             bool retard = false;
//             int  retartedCount = 0;
//             std::cout << TC_BLUE <<
//                     "[O3 RISC-V CMP] -----> start compare"
//                   << TC_DEF << std::endl;
//             while (true){
//                 if (retard && (retartedCount < BELAYED_AFTER_MIS_CMP)){
//                     break;
//                 }
//                 doWorkloadCycle(true);
//                 _slaveRide.doWorkloadCycle(true);
//
//                 if (!retard){
//                     retard = !doCompare(); ///// if belayed  = commpare not corect!
//                 }
//                 if (retard){
//                     if (retartedCount >= BELAYED_AFTER_MIS_CMP){break;}
//                     retartedCount++;
//                 }
//
//
//                 if (isExecFin() && _slaveRide.isExecFin()){
//                     std::cout << TC_GREEN << "slave is equal " << TC_DEF << std::endl;
//                     break;
//                 }else if (isExecFin()){
//                     std::cout << TC_RED << "master is finish but slave not" << TC_DEF << std::endl;
//                     break;
//                 }else if (_slaveRide.isExecFin()){
//                     std::cout << TC_RED << "slave is  finish not like" << TC_DEF << std::endl;
//                     break;
//                 }
//
//             }
//
//             if (retard){
//                 std::cout << TC_RED << "[O3 RISC-V CMP] compare failed see slot writer for the reason mismatch" << TC_DEF << std::endl;
//             }else{
//                 std::cout << TC_GREEN << "[O3 RISC-V CMP] compare pass" << TC_DEF << std::endl;
//             }
//             /////////////////////////////////
//             finalPerfCol();
//         }
//
//     }
//
// }