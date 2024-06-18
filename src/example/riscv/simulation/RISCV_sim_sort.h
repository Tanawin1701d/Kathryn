//
// Created by tanawin on 12/6/2024.
//

#ifndef RISCV_SIM_SORT_H
#define RISCV_SIM_SORT_H
#include "autoSimEvent.h"
#include "RISCV_sim.h"

namespace kathryn{
    namespace riscv{
        class RiscvSimSort: public RiscvSim{
        public:
            uint32_t testFinRegIdx = 31;
            uint32_t _startNumIdx0 = 1 << 20;
            uint32_t _startNumIdx1 = 1 << (20 + 1);
            RiscvSimEvent* _riscvSimEvent = nullptr;

            const int AMT_TEST_CASE = -1;
            int       testCaseId    = 0;

            RiscvSimSort(CYCLE limitCycle,
                                  const std::string& prefix,
                                  std::vector<std::string> testTypes,
                                  Riscv& core,
                                  int amtTestCase
                                  ):
            RiscvSim(limitCycle,prefix,testTypes,core),
            AMT_TEST_CASE(amtTestCase){
                assert(AMT_TEST_CASE > 0);
                requireConSim = false;
            }

            ~RiscvSimSort(){
                delete _riscvSimEvent;
            };

            void describeDef() override;
            void readAssertVal(const std::string& filePath) override{}
            void testRegister() override {}
            void dumpMem(uint32_t startAddr, uint32_t stopAddr);
            int  calculateAmtNumber() const{
                return 1 << (10 + testCaseId);
            }
        };

        class RISCV_SORT_MNG{
        public:
            void start(PARAM& params){

                std::vector<std::string> testTypes = {"sorter"};

                mMod(riscCore, Riscv, false);
                startModelKathryn();
                RiscvSimSort simulator(INT64_MAX,
                                   params["prefix"],
                                   testTypes,
                                   (Riscv &) riscCore,
                                   std::stoi(params["amt"])
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
