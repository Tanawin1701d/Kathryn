//
// Created by tanawin on 1/1/26.
//

#ifndef EXAMPLE_O3_SIMCOMPARE_CTRL_SIMCTRLCOMB_H
#define EXAMPLE_O3_SIMCOMPARE_CTRL_SIMCTRLCOMB_H

#include "simCtrlKride.h"
#include "simCtrlRide.h"
#include "util/fileWriter/slotWriter/wslotWriter.h"

namespace kathryn::o3{

    class CombCtrl : public SimCtrlKride{

    protected:
        const int BELAYED_AFTER_MIS_CMP = 1;
        SimCtrlRide& _slaveRide;
        bool _reqRegTest;

    public:

        explicit CombCtrl(CYCLE                    limitCycle,
                          const std::string&       prefix,
                          std::vector<std::string> testTypes,
                          SimProxyBuildMode        buildMode,
                          SlotWriterBase&          slotWriter,
                          SimState&                state,
                          TopSim&                  topSim,
                          SimCtrlRide&             slaveRide,
                          bool                     reqRegTest

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
                testTypes = { "Tarai" };
                testTypes = { "Cprime" };
            }

            ull limitCycle = stoull(params["limitCycle"]);
            bool reqRegTest = stoi(params["reqRegTest"]) == 1;

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
                            *slaveCore
            );

            CombCtrl simulator(limitCycle,
                            params["prefix"],
                            testTypes,
                            getSPBM(params),
                            slotWriterKride,
                            simState,
                            (TopSim&)o3Top,
                            slaveSimulator,
                            reqRegTest

            );
            simulator.simStart();

            slotWriterKride.finalizeLastWindow();
            slotWriterRide.finalizeLastWindow();

            resetKathryn();
            std::cout << TC_GREEN << "--------------------------------" << std::endl;
        }
    };

}




#endif //EXAMPLE_O3_SIMCOMPARE_CTRL_SIMCTRLCOMB_H