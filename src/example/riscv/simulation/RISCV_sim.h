//
// Created by tanawin on 26/4/2567.
//

#ifndef KATHRYN_RISCV_SIM_H
#define KATHRYN_RISCV_SIM_H

#include <fstream>
#include "kathryn.h"
#include "example/riscv/core/core.h"


namespace kathryn{

    namespace riscv{

        enum PIPE_STAGE{
            RISC_FETCH   = 0,
            RISC_DECODE  = 1,
            RISC_EXECUTE = 2,
            RISC_WB      = 3
        };

        class RiscvSimInterface: public SimInterface{
        private:

            Riscv&      _core;
            const int  AMT_STAGE = 4;
            SlotWriter slotWriter;


        public:

            explicit RiscvSimInterface(CYCLE limitCycle,
                                       std::string vcdFilePath,
                                       std::string profileFilePath,
                                       std::string slotFilePath,
                                       Riscv& core
                                       );

            void describe() override;

            void describeCon() override;

            void recordSlot();

            bool writeSlotIfStall(PIPE_STAGE stageIdx,
                                  FlowBlockPipeBase* pipfb);


            void writeFetchSlot  (FlowBlockPipeBase* pipblock);
            void writeDecodeSlot (FlowBlockPipeBase* pipblock);
            void writeExecuteSlot(FlowBlockPipeBase* pipblock);
            void writeWbSlot     (FlowBlockPipeBase* pipblock);
            void writeReg        (const std::string& prefix,
                                  PIPE_STAGE pipeStage,
                                  RegEle&    regEle);

            void readAssembly(const std::string& filePath);

        };

        class RISCV_MNG{
        public:
            void start(){
                makeMod(riscCore, Riscv, false);
                startModelKathryn();
                RiscvSimInterface simulator(300,
                                            "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTestRiscv.vcd",
                                            "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoRiscv.vcd",
                                            "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/slotAutoRiscv.vcd",
                                            (Riscv&)riscCore
                                            );
                simulator.simStart();


            }
        };

    }

}

#endif //KATHRYN_RISCV_SIM_H
