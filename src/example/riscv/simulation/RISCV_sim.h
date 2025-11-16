//
// Created by tanawin on 26/4/2567.
//

#ifndef KATHRYN_RISCV_SIM_H
#define KATHRYN_RISCV_SIM_H

#include <fstream>
#include "kathryn.h"
#include "slotRecorder.h"
#include "example/riscv/core/core.h"
#include "frontEnd/cmd/paramReader.h"


namespace kathryn{

    namespace riscv{

        class RiscvSim: public SimInterface{
        public:

            Riscv&                   _core;
            const int                AMT_STAGE = 4;
            int                      _curTestCaseIdx = 0;
            SlotWriter               slotWriter;
            std::string              _prefixFolder;
            SlotRecorder             _slotSecorder;
            /////// amount of test
            std::vector<std::string> _testTypes;
            uint32_t                 _regTestVal[AMT_REG]{};


            explicit RiscvSim(CYCLE limitCycle,
                              const std::string& prefix,
                              std::vector<std::string> testTypes,
                              Riscv& core,
                              SimProxyBuildMode buildMode,
                              bool requireLRC = false,
                              bool reqInline  = true,
                              int  opLevel = 3
            );

        public:
            void describeCon() override;


            virtual void readAssembly(const std::string& filePath);
            virtual void readAssertVal(const std::string& filePath);
            void         resetRegister();
            virtual void testRegister();

        };

        class RISCV_MNG{
        public:
            void start(PARAM& params){

                std::vector<std::string> testTypes = {"Imm", "Reg",
                                                      "Ls", "Jump",
                                                      "BranchSc"};

                mMod(riscCore, Riscv, false);
                startModelKathryn();
                RiscvSim simulator(1200,
                                   params["prefix"],
                                   testTypes,
                                   (Riscv &) riscCore,
                                   getSPBM(params)
                );
                ////// start simulate
                simulator.simStart();
                ////// reset system
                resetKathryn();
                std::cout << TC_GREEN << "--------------------------------" << std::endl;

            }
        };

    }

}

#endif //KATHRYN_RISCV_SIM_H
