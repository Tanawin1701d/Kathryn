//
// Created by tanawin on 12/6/2024.
//

#include "riscv_sim_sort.h"

namespace kathryn::riscv{



    void RiscvSimSort::prepare_next_tc(){

        if(test_case_id != -1){
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "end at cycle " << std::to_string(_modelSimEvent->get_amt_l_rsim()) << std::endl;
            std::cout << "process time time: " << elapsed_seconds.count() << "s\n";
            std::cout << "dummping mem 0" << std::endl;
            dump_mem(_startNumIdx0, _startNumIdx0 + (calculate_amt_number() * 4));
            std::cout << "dummping mem 1" << std::endl;
            dump_mem(_startNumIdx1, _startNumIdx1 + (calculate_amt_number() * 4));
            std::cout << "dump finish" << std::endl;

            CYCLE cur_cycle       = get_sim_controller_ptr()->get_cur_cycle();
            CYCLE long_range_cycle = _modelSimEvent->get_amt_l_rsim();

            new UserEvent([&](UserEvent&){
                *rst_wire = 1;
            }, cur_cycle + long_range_cycle, SIM_USER_PRIO_FRONT_CYCLE);
            new UserEvent([&](UserEvent&){
                *rst_wire = 0;
            }, cur_cycle + long_range_cycle + 1, SIM_USER_PRIO_FRONT_CYCLE);

            ////////// send reset signal to next cyclc
        }

        final_perf_col();

        test_case_id++;
        if (test_case_id == AMT_TEST_CASE){
            get_sim_controller_ptr()->stop_sim();
            return;
        }



        std::string test_size_str = std::to_string(test_case_id);
        reset_register();
        read_assembly(_prefixFolder + _testTypes[0] + "/asm" + test_size_str + ".out");
        _flowWriter->renew(_prefixFolder + _testTypes[0]+ "/oprofile"+ test_size_str +".prof");
        for (int i = 0; i < calculate_amt_number(); i++){
            _core.mem_blk._myMem.at((_startNumIdx0 + 4*i)/4)
            .set_var(calculate_amt_number() - i + 10);
        }
        start = std::chrono::steady_clock::now();


    }

    void RiscvSimSort::describe_model_trigger(){
        trig(_core.reg_file[31] == 1, [&](){prepare_next_tc();});
    }

    void RiscvSimSort::describe_def(){
        prepare_next_tc();
        SimInterface::describe_def();
        _modelSimEvent->set_long_run_type(true);

    }


    void RiscvSimSort::describe_con(){}



    void RiscvSimSort::dump_mem(uint32_t start_addr, uint32_t stop_addr){
        FileWriterBase dump_file(_prefixFolder + _testTypes[0] + "/tc" + std::to_string(test_case_id) +
        "_at_"+ std::to_string(start_addr) + ".dump_var");
        for (uint32_t iter = start_addr; iter < stop_addr; iter = iter + 4){
            dump_file.add_data(std::to_string(_core.mem_blk._myMem.at(iter/4).get_val()) + "\n");
        }
    }
}