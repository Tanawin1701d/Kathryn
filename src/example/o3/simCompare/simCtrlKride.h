//
// Created by tanawin on 25/12/25.
//

#ifndef EXAMPLE_O3_SIMCOMPARE_SIMCTRLKRIDE_H
#define EXAMPLE_O3_SIMCOMPARE_SIMCTRLKRIDE_H

#include "simCtrlBase.h"
#include "simStateKride.h"
#include "example/o3/simulation/top.h"

namespace kathryn::o3{

    class SimCtrlKride : public O3SimCtrlBase{
        TopSim& _topSim;
        Core&   _core;

    public:

        explicit SimCtrlKride(CYCLE                    limitCycle,
                              const std::string&       prefix,
                              std::vector<std::string> testTypes,
                              SimProxyBuildMode        buildMode,
                              SlotWriterBase&          slotWriter,
                              SimState&                state,
                              TopSim&                  topSim);
        void  readMem2Fetch() override;
        void  readWriteDataMemGetCmd() override;
        void  readWriteDataMemDoCmd() override;
        void  resetRegister() override;
        void  testRegister() override;
        void  postCycleAction() override;

    };

    class KRIDE_MNG{
    public:
        void start(PARAM& params){

            std::vector<std::string> testTypes = {
                "Imm"       , "Reg"        , "Branch", "BranchSuc",
                "BranchLong", "BranchMidRd", "OverRrf",
                "LoadImm"   , "BranchSc"   , "memOp"
            };
            SlotWriter slotWriter({"MPFT"    , "ARF","RRF"  , "FETCH"  ,"DECODE",
                "DISPATCH", "RSV","ISSUE", "EXECUTE","COMMIT",
                "STBUF"
                   },
                {20       , 40   , 25    , 25       , 30,
                 30       , 35   , 25    , 35       , 25,
                 25},
                std::move(params["prefix"] + testTypes[0] + "/oslot.sl"));

            mMod(o3Top, TopSim, false);

            SimStateKride simState(o3Top.myCore);

            startModelKathryn();
            SimCtrlKride simulator(2500,
                            params["prefix"],
                            testTypes,
                            getSPBM(params),
                            slotWriter,
                            simState,
                            (TopSim&)o3Top
            );
            simulator.simStart();
            resetKathryn();
            std::cout << TC_GREEN << "--------------------------------" << std::endl;
        }
    };


}

#endif //EXAMPLE_O3_SIMCOMPARE_SIMCTRLKRIDE_H