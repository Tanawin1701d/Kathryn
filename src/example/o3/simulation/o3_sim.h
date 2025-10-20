//
// Created by tanawin on 14/10/25.
//

#ifndef SRC_EXAMPLE_O3_SIMULATION_O3_SIM_H
#define SRC_EXAMPLE_O3_SIMULATION_O3_SIM_H

#include <fstream>
#include "kathryn.h"
#include "o3SlotRecoder.h"
#include "example/o3/core/core.h"
#include "frontEnd/cmd/paramReader.h"

    namespace kathryn::o3{
        class O3Sim : public SimInterface{
        public:
            Core& _core;
            const int AMT_STAGE = 5;
            int _curTestCaseIdx = 0;
            SlotWriter slotWriter;
            std::string _prefixFolder;
            O3SlotRecorder _slotRecorder;
            std::vector<std::string> _testTypes;
            uint32_t _regTestVal[REG_NUM]{};

        public:
            explicit O3Sim(CYCLE limitCycle,
                           const std::string& prefix,
                           std::vector<std::string> testTypes,
                           Core& core,
                           SimProxyBuildMode buildMode);

            void          describeCon  () override;
            virtual void  readAssembly (const std::string& filePath);
            virtual void  readAssertVal(const std::string& filePath);
            void          resetRegister();
            virtual void  testRegister ();
        };

        class O3_MNG{
        public:
            void start(PARAM& params){
                std::vector<std::string> testTypes = {"Imm", "Reg", "Branch"
                    //,
                    // "Ls", "Jump",
                    // "BranchSc"
                };

                mMod(o3Core, Core, false);
                startModelKathryn();
                O3Sim simulator(1200,
                                params["prefix"],
                                testTypes,
                                (Core&)o3Core,
                                getSPBM(params)
                );
                simulator.simStart();
                resetKathryn();
                std::cout << TC_GREEN << "--------------------------------" << std::endl;
            }
        };
    }


#endif //SRC_EXAMPLE_O3_SIMULATION_O3_SIM_H
