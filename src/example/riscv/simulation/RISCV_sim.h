//
// Created by tanawin on 26/4/2567.
//

#ifndef KATHRYN_RISCV_SIM_H
#define KATHRYN_RISCV_SIM_H

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

            Riscv      core;
            const int  AMT_STAGE = 4;
            SlotWriter slotWriter;


        public:

            explicit RiscvSimInterface(CYCLE limitCycle,
                                       std::string vcdFilePath,
                                       std::string profileFilePath,
                                       std::string slotFilePath
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

        };

    }

}

#endif //KATHRYN_RISCV_SIM_H
