//
// Created by tanawin on 1/1/26.
//

#ifndef EXAMPLE_O3_SIMCOMPARE_CTRL_SIMCTRLCOMB_H
#define EXAMPLE_O3_SIMCOMPARE_CTRL_SIMCTRLCOMB_H

#ifdef BUILD_RIDECORE

#include "simCtrlKride.h"
#include "simCtrlRide.h"
#include "util/fileWriter/slotWriter/wslotWriter.h"

namespace kathryn::o3{

    class CombCtrl : public SimCtrlKride{

    protected:
        const int BELAYED_AFTER_MIS_CMP = 1;
        SimCtrlRide& _slaveRide;
        bool _reqRegTest;
        ////// slot/debug recording. it must be OFF to measure simulation time fairly:
        ////// the kathryn side dumps extra rsv/dispatch debug slots and an rrf table
        ////// change scan that the ridecore side has no counterpart for.
        bool _recordSlot;

    public:

        explicit CombCtrl(CYCLE                    limitCycle,
                          const std::string&       prefix,
                          std::vector<std::string> testTypes,
                          SimProxyBuildMode        buildMode,
                          SlotWriterBase&          slotWriter,
                          SimState&                state,
                          TopSim&                  topSim,
                          SimCtrlRide&             slaveRide,
                          bool                     reqRegTest,
                          bool                     recordSlot,
                          ResultWriter*            resultWriter = nullptr,
                          int                      simOptLevel  = 3

                          );

        bool doCompare();

        void describeCon() override;

    };


    class COMB_MNG{
    public:
        void start(PARAM& params){

            std::vector<std::string> testTypes = {};

            if (params["workload"] == "standard"){

                testTypes = {
                    "Imm"       , "Reg"        , "Branch", "BranchSuc",
                    "BranchLong", "BranchMidRd", "OverRrf",
                    "LoadImm"   , "BranchSc"   , "memOp"
                };

            }else if (params["workload"] == "cpp"){
                ///testTypes = { "Tarai" };
                testTypes = { "Fibo"  , "Tarai", "Cprime" , "Acker"   , "Hanoi"  ,
                              "Matmul", "Sort3", "Stencil", "Stirling", "Komachi" };
            }

            ull limitCycle = stoull(params["limitCycle"]);
            bool reqRegTest = stoi(params["reqRegTest"]) == 1;
            ////// recordSlot = 0 turns the per cycle slot dump off on BOTH sides.
            ////// it defaults to on so mismatch debugging keeps working.
            bool recordSlot = params["recordSlot"] != "0";
            ////// opt level of the jit built kathryn sim .so. default 3 to match the
            ////// -O3 that cmake gives the verilated ridecore library.
            int simOptLevel = params["simOptLevel"].empty()
                            ? 3 : stoi(params["simOptLevel"]);

            std::vector<std::string> slotColumnNames = {"MPFT"    , "ARF","RRF"  , "FETCH"  ,"DECODE",
                                                        "DISPATCH", "RSV","ISSUE", "EXECUTE","COMMIT", "STBUF"};
            std::vector<int> slotColumnWidth =         {20, 40   , 25, 25, 30,
                                                        30, 35   , 25, 35, 25, 25};

            WSlotWriter slotWriterKride(slotColumnNames, slotColumnWidth,
                params["prefix"] + testTypes[0] + "/oslot_kride.sl",
                2500);
            WSlotWriter slotWriterRide (slotColumnNames, slotColumnWidth,
                params["prefix"] + testTypes[0] + "/oslot_ride.sl",
                2500);

            ResultWriter resultWriterKride(params["prefix"] + testTypes[0] + "/kathryn_kride_result");
            ResultWriter resultWriterRide(params["prefix"] + testTypes[0] + "/verilator_ride_result");

            ///////// build model core
            mMod(o3Top, TopSim, false);
            auto* slaveCore = new Vpipeline();

            ///////// build state tracker
            SimStateKride simState(o3Top.myCore);
            SimStateRide  slaveState(*slaveCore);


            startModelKathryn();

            SimCtrlRide  slaveSimulator(limitCycle,
                            params["prefix"],
                            testTypes,
                            getSPBM(params),
                            slotWriterRide,
                            slaveState,
                            *slaveCore,
                            &resultWriterRide
            );

            CombCtrl simulator(limitCycle,
                            params["prefix"],
                            testTypes,
                            getSPBM(params),
                            slotWriterKride,
                            simState,
                            (TopSim&)o3Top,
                            slaveSimulator,
                            reqRegTest,
                            recordSlot,
                            &resultWriterKride,
                            simOptLevel
            );
            simulator.simStart();

            slotWriterKride.finalizeLastWindow();
            slotWriterRide.finalizeLastWindow();

            resultWriterKride.finalizeTheWriteData();
            resultWriterRide.finalizeTheWriteData();

            resetKathryn();
            std::cout << TC_GREEN << "--------------------------------" << std::endl;
        }
    };

}

#endif


#endif //EXAMPLE_O3_SIMCOMPARE_CTRL_SIMCTRLCOMB_H