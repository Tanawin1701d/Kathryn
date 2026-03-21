//
// Created by tanawin on 15/6/2024.
//

#ifndef SLOTRECORDER_H
#define SLOTRECORDER_H
#include "example/riscv/core/core.h"

#include "util/file_writer/slot_writer/slot_writer.h"

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

            SlotRecorder(SlotWriter*  slot_writer,
                            Riscv*       riscv):
            _slotWriter(slot_writer),
            _riscv(riscv){
                assert(_slotWriter != nullptr);
                assert(_riscv      != nullptr);
            }

            void record_slot();

            bool write_slot_if_pip_stall(PIPE_STAGE2 stage_idx,
                                     PipSimProbe* pipfb) const;
            bool write_slot_if_zync_stall(PIPE_STAGE2 stage_idx,
                                      ZyncSimProb* zync_sim_probe);


            void         write_fetch_slot  (PipSimProbe* pip_sim_probe);
            void         write_decode_slot (PipSimProbe* pip_sim_probe);
            void         write_execute_slot(PipSimProbe* pip_sim_probe);
            void         write_wb_slot     (PipSimProbe* pip_sim_probe);
            virtual void write_mem        (){}
            void         write_reg        (const std::string& prefix,
                                          PIPE_STAGE2 pipe_stage,
                                          OPR_HW&    reg_ele);


        };

    }

}

#endif //SLOTRECORDER_H
