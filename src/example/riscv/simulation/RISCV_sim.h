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

        class RiscvSimSortInterface: public SimInterface{
        private:

            Riscv&      _core;
            const int  AMT_STAGE = 4;
            SlotWriter slotWriter;
            std::string _prefixFolder;
            std::string _testType;
            uint32_t    _regTestVal[AMT_REG];


        public:

            explicit RiscvSimSortInterface(CYCLE limitCycle,
                                           std::string prefix,
                                           std::string testType,
                                           Riscv& core
                                       );

            void describe() override;

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
            void readAssertVal(const std::string& filePath);

        };

        class RISCV_MNG{
        public:
            void start(PARAM& params){

                std::string testTypes[] = {"Imm", "Reg", "Ls", "Jump", "Branch"};


                for (std::string testType: testTypes) {
                    /** test each type*/
                    std::cout << TC_GREEN << "testing riscv instruction >>>> " + testType << std::endl;

                    mMod(riscCore, Riscv, false);
                    startModelKathryn();
                    RiscvSimSortInterface simulator(120,
                                                    params["prefix"],
                                                    testType,
                                                    (Riscv &) riscCore
                    );
                    ////// start simulate
                    simulator.simStart();
                    ////// reset system
                    resetKathryn();
                    std::cout << TC_GREEN << "--------------------------------" << std::endl;

                }


            }
        };

    }

}

#endif //KATHRYN_RISCV_SIM_H
