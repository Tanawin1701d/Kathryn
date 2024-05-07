//
// Created by tanawin on 26/4/2567.
//

#ifndef KATHRYN_RISCV_SIM_SORT_H
#define KATHRYN_RISCV_SIM_SORT_H

#include <fstream>
#include "kathryn.h"
#include "example/riscv/core/core.h"
#include "frontEnd/cmd/paramReader.h"


namespace kathryn{

    namespace riscv{

        enum PIPE_STAGE_SORT{
            RISC_FETCH_SORT   = 0,
            RISC_DECODE_SORT  = 1,
            RISC_EXECUTE_SORT = 2,
            RISC_WB_SORT      = 3,
            RISC_MEM_SORT     = 4
        };

        class RiscvSimInterfaceSort: public SimInterface{
        private:

            Riscv&      _core;
            const int  AMT_STAGE = 4;
            SlotWriter slotWriter;
            std::string _prefixFolder;
            std::string _testType;
            uint32_t    _regTestVal[AMT_REG];

            uint32_t _startNumIdx0 = 1 << 20;
            uint32_t _startNumIdx1 = 1 << (20 + 1);
            uint32_t _numSize      = 4;


        public:

            explicit RiscvSimInterfaceSort(CYCLE limitCycle,
                                       std::string prefix,
                                       std::string testType,
                                       Riscv& core
                                       );

            void describe() override;

            void describeCon() override;

            void recordSlot();

            bool writeSlotIfStall(PIPE_STAGE_SORT stageIdx,
                                  FlowBlockPipeBase* pipfb);


            void writeFetchSlot  (FlowBlockPipeBase* pipblock);
            void writeDecodeSlot (FlowBlockPipeBase* pipblock);
            void writeExecuteSlot(FlowBlockPipeBase* pipblock);
            void writeWbSlot     (FlowBlockPipeBase* pipblock);
            void writeReg        (const std::string& prefix,
                                  PIPE_STAGE_SORT pipeStage,
                                  RegEle&    regEle);
            void writeMem        ();

            void readAssembly(const std::string& filePath);
            void readAssertVal(const std::string& filePath);

        };

        class RISCV_MNG_SORT{
        public:
            void start(PARAM& params){


                    /** test each type*/
                    std::cout << TC_GREEN << "testing riscv instruction SORTING>>>> " << std::endl;

                    mMod(riscCore, Riscv, false);
                    startModelKathryn();
                    RiscvSimInterfaceSort simulator(20000,
                                                params["prefix"],
                                                "sorter",
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
