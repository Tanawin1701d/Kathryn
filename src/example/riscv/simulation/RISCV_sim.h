//
// Created by tanawin on 26/4/2567.
//

#ifndef KATHRYN_RISCV_SIM_H
#define KATHRYN_RISCV_SIM_H

#include <fstream>
#include "kathryn.h"
#include "example/riscv/core/core.h"
#include "frontEnd/cmd/paramReader.h"


namespace kathryn{

    namespace riscv{

        enum PIPE_STAGE2{
            RISC_FETCH   = 0,
            RISC_DECODE  = 1,
            RISC_EXECUTE = 2,
            RISC_WB      = 3
        };

        class RiscvSim: public SimInterface{
        private:

            Riscv&                   _core;
            const int                AMT_STAGE = 4;
            int                      _curTestCaseIdx = 0;
            SlotWriter               slotWriter;
            std::string              _prefixFolder;
            /////// amount of test
            std::vector<std::string> _testTypes;
            uint32_t                 _regTestVal[AMT_REG]{};


        public:

            explicit RiscvSim(CYCLE limitCycle,
                              const std::string& prefix,
                              std::vector<std::string> testTypes,
                              Riscv& core
                                       );

            void describeCon() override;

            void recordSlot();

            bool writeSlotIfStall(PIPE_STAGE2 stageIdx,
                                  FlowBlockPipeBase* pipfb);


            void writeFetchSlot  (FlowBlockPipeBase* pipblock);
            void writeDecodeSlot (FlowBlockPipeBase* pipblock);
            void writeExecuteSlot(FlowBlockPipeBase* pipblock);
            void writeWbSlot     (FlowBlockPipeBase* pipblock);
            void writeReg        (const std::string& prefix,
                                  PIPE_STAGE2 pipeStage,
                                  RegEle&    regEle);

            void readAssembly(const std::string& filePath);
            void resetRegister();
            void readAssertVal(const std::string& filePath);

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
                                   (Riscv &) riscCore
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
