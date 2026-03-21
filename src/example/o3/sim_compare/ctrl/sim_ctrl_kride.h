//
// Created by tanawin on 25/12/25.
//

#ifndef EXAMPLE_O3_SIMCOMPARE_SIMCTRLKRIDE_H
#define EXAMPLE_O3_SIMCOMPARE_SIMCTRLKRIDE_H

#include "sim_ctrl_base.h"
#include "../sim_state_kride.h"
#include "example/o3/simulation/top.h"

namespace kathryn::o3{

    class SimCtrlKride : public O3SimCtrlBase,
                         public SimInterface{
        TopSim& _topSim;
        Core&   _core;

    public:

        explicit SimCtrlKride(CYCLE                    limit_cycle,
                              const std::string&       prefix,
                              std::vector<std::string> test_types,
                              SimProxyBuildMode        build_mode,
                              SlotWriterBase&          slot_writer,
                              SimState&                state,
                              TopSim&                  top_sim,
                              ResultWriter*            result_writer = nullptr);

        void  describe_con  () override;

        void do_workload_init (int cur_test_case_idx, bool req_reg_test) override;
        void do_workload_cycle(bool record_this_cycle) override;

        void  read_mem2_fetch() override;
        void  read_write_data_mem_get_cmd() override;
        void  read_write_data_mem_do_cmd() override;
        void  reset_register() override;
        void  test_register() override;
        void  post_cycle_action() override;

    };

    class KRIDE_MNG{
    public:
        void start(PARAM& params){

            std::vector<std::string> test_types = {
                "Imm"       , "Reg"        , "Branch", "BranchSuc",
                "BranchLong", "BranchMidRd", "OverRrf",
                "LoadImm"   , "BranchSc"   , "mem_op"
            };
            SlotWriter slot_writer({"MPFT"    , "ARF","RRF"  , "FETCH"  ,"DECODE",
                "DISPATCH", "RSV","ISSUE", "EXECUTE","COMMIT",
                "STBUF"
                   },
                {20       , 40   , 25    , 25       , 30,
                 30       , 35   , 25    , 35       , 25,
                 25},
                std::move(params["prefix"] + test_types[0] + "/oslot.sl"));

            m_mod(o3Top, TopSim, false);

            SimStateKride sim_state(o3Top.my_core);

            start_model_kathryn();
            SimCtrlKride simulator(2500,
                            params["prefix"],
                            test_types,
                            get_spbm(params),
                            slot_writer,
                            sim_state,
                            (TopSim&)o3Top
            );
            simulator.sim_start();
            reset_kathryn();
            std::cout << TC_GREEN << "--------------------------------" << std::endl;
        }
    };


}

#endif //EXAMPLE_O3_SIMCOMPARE_SIMCTRLKRIDE_H