//
// Created by tanawin on 1/6/2024.
//

#include "proxy_event_base.h"
#include "params/sim_param_type.h"


#include "utility"

namespace kathryn{
    ProxySimEventBase::ProxySimEventBase():
        EventBase(0, SIM_MODEL_PRIO, false),
        VCD_REC_POL(MDE_REC_SKIP),
        _vcdWriter(nullptr){
#ifdef MODELCOMPILEVB
    std::cout << "constructor of proxy sim event base" << std::endl;
#endif
    }

    ProxySimEventBase::~ProxySimEventBase(){ delete _vcdWriter; }

    void ProxySimEventBase::event_warm_up(){
        start_register_call_back();
        _vcdWriter->add_new_var(VST_REG, CLK_SIGNAL, {0, 1});
        if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_USER)){
            start_vcd_dec_var_user();
        }
        if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
            start_vcd_dec_var_internal();
        }
    }

    void ProxySimEventBase::sim_start_long_run_cycle(){
        assert(_isLongRangeSim);
        std::cout << "start long range run with amount lim cycle "<<
            _amtLimitLongRangeCycle << std::endl;
        amt_lr_sim = main_sim();

        //////std::cout << "long range get  amt_lr_sim "<< amt_lr_sim << std::endl;
    }

    /*
     *  single cycle simulation
     */

    void ProxySimEventBase::sim_start_cur_cycle_neg(){
        if (is_long_rage_sim()){return;}
        start_main_op_ele_sim_neg();
    }

    void ProxySimEventBase::sim_start_cur_cycle_pos(){
        if (is_long_rage_sim()){return;}
        start_main_op_ele_sim_pos();
    }

    void ProxySimEventBase::cur_cycle_collect_data_neg(){

        if (is_long_rage_sim()){return;}
        write_vcd_signal();
    }
    void ProxySimEventBase::cur_cycle_collect_data_pos(){
        if (is_long_rage_sim()){return;}
        write_vcd_signal();
        start_perf_col();
    }

    void ProxySimEventBase::sim_start_next_cycle_neg(){
        if (is_long_rage_sim()){return;}
        start_finalize_ele_sim_neg();
        //////// sim register change exact register
           ///////////// do not wory about register simulation will get false new
           ///data from memory if there is update from memory to register because
           /// mem_ele_holder will provide temporary data to register simulation
    }

    void ProxySimEventBase::sim_start_next_cycle_pos(){
        if (is_long_rage_sim()){return;}
        start_finalize_ele_sim_pos();
    }

    EventBase* ProxySimEventBase::gen_next_event(){
        ///////////////std::cout << _targetCycle << std::endl;
        if (is_long_rage_sim()){
            _targetCycle += get_amt_l_rsim();
        }else{
            _targetCycle++;
        }
        return this;
    }

    void ProxySimEventBase::write_vcd_signal(){

        if ((VCD_REC_POL == MDE_REC_BOTH) |
            (VCD_REC_POL == MDE_REC_ONLY_USER) |
            (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)
        ){
            assert(_vcdWriter !=nullptr);

            if (clk_signal == 1){ ///// the old value is 1
                _vcdWriter->add_new_time_stamp(get_cur_cycle() * 10 + 5);
                clk_signal = 0;
            }else{ ///// the old value is 0
                ///// the new value is 1
                _vcdWriter->add_new_time_stamp(get_cur_cycle() * 10);
                clk_signal = 1;
            }
            _vcdWriter->add_new_value(CLK_SIGNAL, clk_signal);

        }

        if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_USER)){
            //std::cout << "add vcd user col" << std::endl;
            start_vcd_col_user();
            //std::cout << "add vcd fin" << std::endl;
        }

        if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
            start_vcd_col_internal();
        }

        // if ((VCD_REC_POL == MDE_REC_BOTH) |
        //     (VCD_REC_POL == MDE_REC_ONLY_USER) |
        //     (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)
        // ){
        //     _vcdWriter->add_new_time_stamp(get_cur_cycle() * 10 + 5);
        //     clk_signal = 0;
        //     _vcdWriter->add_new_value(CLK_SIGNAL, clk_signal);
        // }

    }

}
