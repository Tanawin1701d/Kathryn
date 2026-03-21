//
// Created by tanawin on 12/6/2024.
//

#ifndef RISCV_SIM_SORT_H
#define RISCV_SIM_SORT_H
#include "riscv_sim.h"

namespace kathryn{
    namespace riscv{
        class RiscvSimSort: public RiscvSim{
        public:
            uint32_t test_fin_reg_idx = 31;
            uint32_t _startNumIdx0 = 1 << 20;
            uint32_t _startNumIdx1 = 1 << (20 + 1);

            const int AMT_TEST_CASE = -1;
            int       test_case_id    = -1;

            std::chrono::time_point<std::chrono::steady_clock> start;

            RiscvSimSort(CYCLE limit_cycle,
                                  const std::string& prefix,
                                  std::vector<std::string> test_types,
                                  Riscv& core,
                                  int amt_test_case,
                                  SimProxyBuildMode build_mode
                                  ):
            RiscvSim(limit_cycle,prefix,test_types,core, build_mode, false, true, 3),
            AMT_TEST_CASE(amt_test_case){
                assert(AMT_TEST_CASE > 0);
                require_con_sim = false;
                enable_lrc(); ///// sim sort we enable LRC to make it run through
            }

            void prepare_next_tc();

            /////// discription
            void describe_model_trigger() override;
            void describe_def() override;
            void describe_con() override;

            void read_assert_val(const std::string& file_path) override{}
            void test_register() override {}
            void dump_mem(uint32_t start_addr, uint32_t stop_addr);
            int  calculate_amt_number() const{
                return 1 << (10 + test_case_id);
            }
        };

        class RISCV_SORT_MNG{
        public:
            void start(PARAM& params){

                std::vector<std::string> test_types = {"sorter"};

                m_mod(risc_core, Riscv, false);
                start_model_kathryn();
                RiscvSimSort simulator(INT64_MAX,
                                   params["prefix"],
                                   test_types,
                                   (Riscv &) risc_core,
                                   std::stoi(params["amt"]),
                                   get_spbm(params)
                );
                ////// start simulate
                simulator.sim_start();
                ////// reset system
                reset_kathryn();
                std::cout << TC_GREEN << "--------------------------------" << std::endl;

            }
        };

    }
}

#endif //RISCV_SIM_SORT_H
