//
// Created by tanawin on 15/6/2024.
//

#ifndef SLOTRECORDER_H
#define SLOTRECORDER_H
#include "example/riscv/core/core.h"

#include "util/fileWriter/slotWriter/slotWriter.h"

namespace kathryn{

    namespace riscv{

        enum PIPE_STAGE2{
            RISC_FETCH   = 0,
            RISC_DECODE  = 1,
            RISC_EXECUTE = 2,
            RISC_WB      = 3,
            RISC_MEM     = 4
        };


        class SlotRecorder{
        public:
            SlotWriter*  _slotWriter = nullptr;
            Riscv*       _riscv     = nullptr;

            SlotRecorder(SlotWriter*  slotWriter,
                            Riscv*       riscv):
            _slotWriter(slotWriter),
            _riscv(riscv){
                assert(_slotWriter != nullptr);
                assert(_riscv      != nullptr);
            }

            void recordSlot();

            bool writeSlotIfStall(PIPE_STAGE2 stageIdx,
                                  FlowBlockPipeBase* pipfb);


            void         writeFetchSlot  (FlowBlockPipeBase* pipblock);
            void         writeDecodeSlot (FlowBlockPipeBase* pipblock);
            void         writeExecuteSlot(FlowBlockPipeBase* pipblock);
            void         writeWbSlot     (FlowBlockPipeBase* pipblock);
            virtual void writeMem        (){}
            void         writeReg        (const std::string& prefix,
                                          PIPE_STAGE2 pipeStage,
                                          OPR_HW&    regEle);


        };

    }

}

#endif //SLOTRECORDER_H
