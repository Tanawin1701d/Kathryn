//
// Created by tanawin on 14/10/25.
//

#ifndef SRC_EXAMPLE_O3_SIMULATION_O3_SIM_H
#define SRC_EXAMPLE_O3_SIMULATION_O3_SIM_H

#include "fstream"
#include "kathryn.h"
#include "o3_slot_recoder.h"
#include "top.h"
#include "front_end/cmd/param_reader.h"

    namespace kathryn::o3{
        class O3Sim : public SimInterface{
        public:
            TopSim& _top;
            Core& _core;
            const int AMT_STAGE = 5;
            int _curTestCaseIdx = 0;
            SlotWriter slot_writer;
            std::string _prefixFolder;
            O3SlotRecorder _slotRecorder;
            std::vector<std::string> _testTypes;
            uint32_t _imem      [IMEM_ROW]{};
            uint32_t _dmem      [DMEM_ROW]{};
            uint32_t _regTestVal[REG_NUM] {};

            bool     last_dmem_enable = false; //// enabler
            bool     last_dmem_read   = true;  //// used if enabler is true
            uint32_t last_dmem_addr   = 0;
            uint32_t last_dmem_data   = 0;

        public:
            explicit O3Sim(CYCLE limit_cycle,
                           const std::string& prefix,
                           std::vector<std::string> test_types,
                           TopSim& top,
                           SimProxyBuildMode build_mode);

            void          describe_con  () override;
            ////// memory management for each cycle
            void          read_mem2_fetch(); //// it has to place at the begin of cycle
            void          reset_dmem();
            void          read_write_data_mem_get_cmd(); //// it has to place at the end of cycle
            void          read_write_data_mem_do_cmd();  //// it has to place at the bigin of cycle

            ////// memory initialization
            virtual void  read_assembly (const std::string& file_path);


            virtual void  read_assert_val(const std::string& file_path);
            void          reset_register();
            virtual void  test_register ();
        };

        class O3_MNG{
        public:
            void start(PARAM& params){
                std::vector<std::string> test_types = {
                    "Imm"       , "Reg"        , "Branch", "BranchSuc",
                    "BranchLong", "BranchMidRd", "OverRrf",
                    "LoadImm"   , "BranchSc"   , "mem_op"
                };

                m_mod(o3Top, TopSim, false);
                start_model_kathryn();
                O3Sim simulator(2500,
                                params["prefix"],
                                test_types,
                                (TopSim&)o3Top,
                                get_spbm(params)
                );
                simulator.sim_start();
                reset_kathryn();
                std::cout << TC_GREEN << "--------------------------------" << std::endl;
            }
        };
    }


#endif //SRC_EXAMPLE_O3_SIMULATION_O3_SIM_H
