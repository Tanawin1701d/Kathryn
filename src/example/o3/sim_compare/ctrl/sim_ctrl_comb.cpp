//
// Created by tanawin on 1/1/26.
//


#ifdef BUILD_RIDECORE

#include "sim_ctrl_comb.h"

#include "utility"

namespace kathryn::o3{


    CombCtrl::CombCtrl(CYCLE                    limit_cycle,
                       const std::string&       prefix,
                       std::vector<std::string> test_types,
                       SimProxyBuildMode        build_mode,
                       SlotWriterBase&          slot_writer,
                       SimState&                state,
                       TopSim&                  top_sim,
                       SimCtrlRide&             slave_ride,
                       bool                     req_reg_test,
                       ResultWriter*            result_writer
    ):
    SimCtrlKride(limit_cycle,
                 prefix,
                 std::move(test_types),
                 build_mode,
                 slot_writer,
                 state,
                 top_sim,
                 result_writer),
    _slaveRide  (slave_ride),
    _reqRegTest (req_reg_test)
    {}

    bool CombCtrl::do_compare(){
        bool compare_valid = _state.compare(_slaveRide._state);
        compare_valid &= compare_mem_op(_slaveRide);
        return compare_valid;

    }

    void CombCtrl::describe_con(){

        std::vector<int> error_indexs;

        for (; _curTestCaseIdx < _testTypes.size(); _curTestCaseIdx++){
            std::cout << std::endl
                      << std::endl
                      << std::endl;
            std::cout << TC_BLUE << "[O3 RISC-V CMP] test type is "
                      << _testTypes[_curTestCaseIdx]
                      << TC_DEF << std::endl;

            ////// init kride and ride
            do_workload_init(_curTestCaseIdx, _reqRegTest);
            do_workload_cycle(false);
            _slaveRide.do_workload_init(_curTestCaseIdx, _reqRegTest);
            //////// iterate for 100 cycle
            bool retard = false;
            int  retarted_count = 0;
            std::cout << TC_BLUE <<
                    "[O3 RISC-V CMP] -----> start compare"
                  << TC_DEF << std::endl;
            while (true){
                if (retard && (retarted_count < BELAYED_AFTER_MIS_CMP)){
                    break;
                }
                do_workload_cycle(true);
                _slaveRide.do_workload_cycle(true);

                if (!retard){
                    retard = !do_compare(); ///// if belayed  = commpare not corect!
                }
                if (retard){
                    if (retarted_count >= BELAYED_AFTER_MIS_CMP){break;}
                    retarted_count++;
                }


                if (is_exec_fin() && _slaveRide.is_exec_fin()){
                    std::cout << TC_GREEN << "slave is equal " << TC_DEF << std::endl;
                    break;
                }else if (is_exec_fin()){
                    std::cout << TC_RED << "master is finish but slave not" << TC_DEF << std::endl;
                    break;
                }else if (_slaveRide.is_exec_fin()){
                    std::cout << TC_RED << "slave is  finish not like" << TC_DEF << std::endl;
                    break;
                }
                if (cycle_cnt % 10000 == 0){
                    std::cout << TC_BLUE << "[O3 RISC-V CMP] -----> computing cycle " << cycle_cnt << TC_DEF << std::endl;
                }

                ////// increase cycle counter
                inc_cycle_cnt();
                _slaveRide.inc_cycle_cnt();
            }
            std::cout << TC_BLUE << "[O3 RISC-V CMP] -----> sim done in " << cycle_cnt << " cycles" << TC_DEF << std::endl;

            if (retard){
                std::cout << TC_RED << "[O3 RISC-V CMP] compare failed see slot writer for the reason mismatch" << TC_DEF << std::endl;
                error_indexs.push_back(_curTestCaseIdx);
            }else{
                std::cout << TC_GREEN << "[O3 RISC-V CMP] compare pass" << TC_DEF << std::endl;
            }
            /////////////////////////////////

            if (_reqRegTest){
                test_register();
                _slaveRide.test_register();
            }
            final_perf_col();
        }
        do_workload_exit();
        _slaveRide.do_workload_exit();

        if (error_indexs.empty()){
            std::cout << TC_RED << "[O3 RISC-V CMP] all tests passes" << TC_DEF << std::endl;
        }else{
            for (int error_idx: error_indexs){
                std::cout << TC_RED << "[O3 RISC-V CMP] error in test case " << error_idx << TC_DEF << std::endl;
            }
        }
    }
}

#endif //BUILD_RIDECORE