//
// Created by tanawin on 6/1/26.
//

#ifndef EXAMPLE_O3_SIMCOMPARE_CTRL_SIMCTRLCOMBCXX_H
#define EXAMPLE_O3_SIMCOMPARE_CTRL_SIMCTRLCOMBCXX_H
//
// #include "simCtrlComb.h"
//
// namespace kathryn::o3{
//
//     class CombCtrlCxx: public CombCtrl{
//     public:
//         explicit CombCtrlCxx(CYCLE                    limitCycle,
//                              const std::string&       prefix,
//                              std::vector<std::string> testTypes,
//                              SimProxyBuildMode        buildMode,
//                              SlotWriterBase&          slotWriter,
//                              SimState&                state,
//                              TopSim&                  topSim,
//                              SimCtrlRide&             slaveRide
//                              );
//
//         void describeCon() override;
//
//     };
//
//     class COMB_CXX_MNG{
//     public:
//         void start(PARAM& params){
//
//             std::vector<std::string> testTypes = {
//                 "Fibo"
//             };
//             std::vector<std::string> slotColumnNames = {"MPFT"    , "ARF","RRF"  , "FETCH"  ,"DECODE",
//                                                         "DISPATCH", "RSV","ISSUE", "EXECUTE","COMMIT", "STBUF"};
//             std::vector<int> slotColumnWidth =         {20, 40   , 25, 25, 30,
//                                                         30, 35   , 25, 35, 25, 25};
//
//             SlotWriter slotWriterKride(slotColumnNames, slotColumnWidth,
//                 std::move(params["prefix"] + testTypes[0] + "/oslot_kride.sl"));
//             SlotWriter slotWriterRide (slotColumnNames, slotColumnWidth,
//                 std::move(params["prefix"] + testTypes[0] + "/oslot_ride.sl"));
//
//             ///////// build model core
//             mMod(o3Top, TopSim, false);
//             auto* slaveCore = new Vpipeline();
//
//             ///////// build state tracker
//             SimStateKride simState(o3Top.myCore);
//             SimStateRide  slaveState(*slaveCore);
//
//
//             startModelKathryn();
//
//             SimCtrlRide  slaveSimulator(10000,
//                             params["prefix"],
//                             testTypes,
//                             getSPBM(params),
//                             slotWriterRide,
//                             slaveState,
//                             *slaveCore
//             );
//
//             CombCtrlCxx simulator(10000,
//                                   params["prefix"],
//                                   testTypes,
//                                   getSPBM(params),
//                                   slotWriterKride,
//                                   simState,
//                                   (TopSim&)o3Top,
//                                   slaveSimulator
//             );
//             simulator.simStart();
//             resetKathryn();
//             std::cout << TC_GREEN << "--------------------------------" << std::endl;
//         }
//     };
//
// }

#endif //KATHRYN_SIMCTRLCOMBCXX_H
