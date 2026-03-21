//
// Created by tanawin on 1/1/26.
//

#ifndef EXAMPLE_O3_SIMCOMPARE_CTRL_SIMCTRLCOMB_H
#define EXAMPLE_O3_SIMCOMPARE_CTRL_SIMCTRLCOMB_H

#ifdef BUILD_RIDECORE

#include "sim_ctrl_kride.h"
#include "sim_ctrl_ride.h"
#include "util/file_writer/slot_writer/wslot_writer.h"

namespace kathryn::o3{

    class CombCtrl : public SimCtrlKride{

    protected:
        const int BELAYED_AFTER_MIS_CMP = 1;
        SimCtrlRide& _slaveRide;
        bool _reqRegTest;

    public:

        explicit CombCtrl(CYCLE                    limit_cycle,
                          const std::string&       prefix,
                          std::vector<std::string> test_types,
                          SimProxyBuildMode        build_mode,
                          SlotWriterBase&          slot_writer,
                          SimState&                state,
                          TopSim&                  top_sim,
                          SimCtrlRide&             slave_ride,
                          bool                     req_reg_test,
                          ResultWriter*            result_writer = nullptr

                          );

        bool do_compare();

        void describe_con() override;

    };


    class COMB_MNG{
    public:
        void start(PARAM& params){

            std::vector<std::string> test_types = {};

            if (params["workload"] == "standard"){

                test_types = {
                    "Imm"       , "Reg"        , "Branch", "BranchSuc",
                    "BranchLong", "BranchMidRd", "OverRrf",
                    "LoadImm"   , "BranchSc"   , "mem_op"
                };

            }else if (params["workload"] == "cpp"){
                ///test_types = { "Tarai" };
                test_types = { "Fibo"  , "Tarai", "Cprime" , "Acker"   , "Hanoi"  ,
                              "Matmul", "Sort3", "Stencil", "Stirling", "Komachi" };
            }

            ull limit_cycle = stoull(params["limit_cycle"]);
            bool req_reg_test = stoi(params["req_reg_test"]) == 1;

            std::vector<std::string> slot_column_names = {"MPFT"    , "ARF","RRF"  , "FETCH"  ,"DECODE",
                                                        "DISPATCH", "RSV","ISSUE", "EXECUTE","COMMIT", "STBUF"};
            std::vector<int> slot_column_width =         {20, 40   , 25, 25, 30,
                                                        30, 35   , 25, 35, 25, 25};

            WSlotWriter slot_writer_kride(slot_column_names, slot_column_width,
                params["prefix"] + test_types[0] + "/oslot_kride.sl",
                2500);
            WSlotWriter slot_writer_ride (slot_column_names, slot_column_width,
                params["prefix"] + test_types[0] + "/oslot_ride.sl",
                2500);

            ResultWriter result_writer_kride(params["prefix"] + test_types[0] + "/kathryn_kride_result");
            ResultWriter result_writer_ride(params["prefix"] + test_types[0] + "/verilator_ride_result");

            ///////// build model core
            m_mod(o3Top, TopSim, false);
            auto* slave_core = new Vpipeline();

            ///////// build state tracker
            SimStateKride sim_state(o3Top.my_core);
            SimStateRide  slave_state(*slave_core);


            start_model_kathryn();

            SimCtrlRide  slave_simulator(limit_cycle,
                            params["prefix"],
                            test_types,
                            get_spbm(params),
                            slot_writer_ride,
                            slave_state,
                            *slave_core,
                            &result_writer_ride
            );

            CombCtrl simulator(limit_cycle,
                            params["prefix"],
                            test_types,
                            get_spbm(params),
                            slot_writer_kride,
                            sim_state,
                            (TopSim&)o3Top,
                            slave_simulator,
                            req_reg_test,
                            &result_writer_kride
            );
            simulator.sim_start();

            slot_writer_kride.finalize_last_window();
            slot_writer_ride.finalize_last_window();

            result_writer_kride.finalize_the_write_data();
            result_writer_ride.finalize_the_write_data();

            reset_kathryn();
            std::cout << TC_GREEN << "--------------------------------" << std::endl;
        }
    };

}

#endif


#endif //EXAMPLE_O3_SIMCOMPARE_CTRL_SIMCTRLCOMB_H