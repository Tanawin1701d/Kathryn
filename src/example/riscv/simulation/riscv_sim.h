//
// Created by tanawin on 26/4/2567.
//

#ifndef KATHRYN_RISCV_SIM_H
#define KATHRYN_RISCV_SIM_H

#include "fstream"
#include "kathryn.h"
#include "slot_recorder.h"
#include "example/riscv/core/core.h"
#include "front_end/cmd/param_reader.h"


namespace kathryn{

    namespace riscv{

        class RiscvSim: public SimInterface{
        public:

            Riscv&                   _core;
            const int                AMT_STAGE = 4;
            int                      _curTestCaseIdx = 0;
            SlotWriter               slot_writer;
            std::string              _prefixFolder;
            SlotRecorder             _slotSecorder;
            /////// amount of test
            std::vector<std::string> _testTypes;
            uint32_t                 _regTestVal[AMT_REG]{};


            explicit RiscvSim(CYCLE limit_cycle,
                              const std::string& prefix,
                              std::vector<std::string> test_types,
                              Riscv& core,
                              SimProxyBuildMode build_mode,
                              bool require_lrc = false,
                              bool req_inline  = true,
                              int  op_level = 3
            );

        public:
            void describe_con() override;


            virtual void read_assembly(const std::string& file_path);
            virtual void read_assert_val(const std::string& file_path);
            void         reset_register();
            virtual void test_register();

        };

        class RISCV_MNG{
        public:
            void start(PARAM& params){

                std::vector<std::string> test_types = {"Imm", "Reg",
                                                      "Ls", "Jump",
                                                      "BranchSc"};

                m_mod(risc_core, Riscv, false);
                start_model_kathryn();
                RiscvSim simulator(1200,
                                   params["prefix"],
                                   test_types,
                                   (Riscv &) risc_core,
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

#endif //KATHRYN_RISCV_SIM_H
