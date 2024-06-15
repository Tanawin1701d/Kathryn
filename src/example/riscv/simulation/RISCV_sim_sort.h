//
// Created by tanawin on 12/6/2024.
//

#ifndef RISCV_SIM_SORT_H
#define RISCV_SIM_SORT_H
#include "RISCV_sim.h"

namespace kathryn{
    namespace riscv{
        class RiscvSimSort: public RiscvSim{
        public:
            uint32_t testFinRegIdx = 31;
            uint32_t _startNumIdx0 = 1 << 20;
            uint32_t _startNumIdx1 = 1 << (20 + 1);
            uint32_t _numSize      = 1024;

            RiscvSimSort(CYCLE limitCycle,
                                  const std::string& prefix,
                                  std::vector<std::string> testTypes,
                                  Riscv& core):
            RiscvSim(limitCycle,prefix,testTypes,core){}

            void describeCon() override;

            void writeMem        () override;

            void readAssembly (const std::string& filePath) override;
            void readAssertVal(const std::string& filePath) override{};
            void testRegister() override {}
            void dumpMem(uint32_t startAddr, uint32_t stopAddr);
        };

        class RISCV_SORT_MNG{
        public:
            void start(PARAM& params){

                std::vector<std::string> testTypes = {"sorter"};

                mMod(riscCore, Riscv, false);
                startModelKathryn();
                RiscvSimSort simulator(800000 + 5,
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

#endif //RISCV_SIM_SORT_H
