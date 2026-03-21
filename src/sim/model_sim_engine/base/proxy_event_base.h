//
// Created by tanawin on 1/6/2024.
//

#ifndef MODEL_COMPILE_PROXY_EVENT_BASE_H
#define MODEL_COMPILE_PROXY_EVENT_BASE_H

#include "unordered_map"
#include "utility"

#include "params/sim_param_type.h"
#include "sim/logic_rep/val_rep.h"
#include "sim/event/event_base.h"
#include "sim/sim_res_writer/sim_res_writer.h"

namespace kathryn{


    class ProxySimEventBase: public EventBase{
    protected:
        const std::string   CLK_SIGNAL  = "CLK";
        ull                 clk_signal   = 0;
        MODULE_VCD_REC_POL  VCD_REC_POL = MDE_REC_SKIP;
        VcdWriter*          _vcdWriter = nullptr;

        std::unordered_map<std::string, uint8_t*>    call_back8; //// for mem block use start point
        std::unordered_map<std::string, uint16_t*>   call_back16; //// for mem block use start point
        std::unordered_map<std::string, uint32_t*>   call_back32; //// for mem block use start point
        std::unordered_map<std::string, uint64_t*>   call_back64; //// for mem block use start point
        std::unordered_map<std::string, UintX_Base*> call_back64_m;

        std::unordered_map<std::string, uint64_t*> call_back_perf;
        std::unordered_map<std::string, uint8_t*> call_back_perf_curbit;

        CYCLE amt_lr_sim = 0; ///// amount of long run in each [sim_start_long_run_cycle]

        /////// for flow collection we will let flow model handle this instead
        /// we just collect only data that flow block use

    public:
        ////// constructor
        explicit ProxySimEventBase();
        ~ProxySimEventBase() override;
        ////// sim base
        void event_warm_up();
        virtual void int_code_warm_up() = 0; ///// internal code warm up
        ///// long cycle sim
        void sim_start_long_run_cycle() override;
        ///// single cycle sim
        void sim_start_cur_cycle_neg   () override;
        void sim_start_cur_cycle_pos   () override;
        void cur_cycle_collect_data_neg() override;
        void cur_cycle_collect_data_pos() override;
        void sim_start_next_cycle_neg  () override;
        void sim_start_next_cycle_pos  () override;

        void sim_exit_cur_cycle()      override{}
        EventBase* gen_next_event()   override;
        ////// sim base
        void write_vcd_signal();

        bool need_to_delete()        override{return false;}

        void set_long_run_type    (bool is_long_run)                {_isLongRangeSim = is_long_run;}
        void set_vcd_write_pol    (MODULE_VCD_REC_POL vcd_rec_pol){VCD_REC_POL = vcd_rec_pol;}
        void set_vcd_writer      (VcdWriter*         vcd_writer  ){_vcdWriter   = vcd_writer;  }
        ///////// register callback 8
        void register_to_call_back(const std::string& cb_name, uint8_t& val){
            register_to_call_back(cb_name, &val);
        }
        void register_to_call_back(const std::string& cb_name, uint8_t* val){
            assert(call_back8.find(cb_name) == call_back8.end());
            call_back8.insert({cb_name, val});
        }
        ///////// register callback 16
        void register_to_call_back(const std::string& cb_name, uint16_t& val){
            register_to_call_back(cb_name, &val);
        }
        void register_to_call_back(const std::string& cb_name, uint16_t* val){
            assert(call_back16.find(cb_name) == call_back16.end());
            call_back16.insert({cb_name, val});
        }
        ///////// register callback 32
        void register_to_call_back(const std::string& cb_name, uint32_t& val){
            register_to_call_back(cb_name, &val);
        }
        void register_to_call_back(const std::string& cb_name, uint32_t* val){
            assert(call_back32.find(cb_name) == call_back32.end());
            call_back32.insert({cb_name, val});
        }
        ///////// register callback 64
        void register_to_call_back(const std::string& cb_name, uint64_t& val){
            register_to_call_back(cb_name, &val);
        }
        void register_to_call_back(const std::string& cb_name, uint64_t* val){
            assert(call_back64.find(cb_name) == call_back64.end());
            call_back64.insert({cb_name, val});
        }

        ///////// register callback 64M
        void register_to_call_back(const std::string& cb_name, UintX_Base& val){
            register_to_call_back(cb_name, &val);
        }
        void register_to_call_back(const std::string& cb_name, UintX_Base* val){
            assert(call_back64_m.find(cb_name) == call_back64_m.end());
            call_back64_m.insert({cb_name, val});
        }

        void register_to_call_back_perf(const std::string& cb_name, uint64_t& val){
            assert(call_back_perf.find(cb_name) == call_back_perf.end());
            call_back_perf.insert({cb_name, &val});
        }

        void register_to_call_back_perf(const std::string& cb_name, uint8_t& val){
            assert(call_back_perf_curbit.find(cb_name) == call_back_perf_curbit.end());
            call_back_perf_curbit.insert({cb_name, &val});
        }

        ValRepBase get_val    (const std::string& global_name){

            if (call_back8.find(global_name) != call_back8.end()){
                return {sizeof(uint8_t), call_back8.find(global_name)->second};
            }
            if (call_back16.find(global_name) != call_back16.end()){
                return {sizeof(uint16_t), call_back16.find(global_name)->second};
            }
            if (call_back32.find(global_name) != call_back32.end()){
                return {sizeof(uint32_t), call_back32.find(global_name)->second};
            }
            if (call_back64.find(global_name) != call_back64.end()){
                return {sizeof(uint64_t), call_back64.find(global_name)->second};
            }
            if (call_back64_m.find(global_name) != call_back64_m.end()){
                return {sizeof(uint64_t), call_back64_m.find(global_name)->second->get_data_base_ptr()};
            }
            assert(false);

        }

        ValRepBase get_val_perf(const std::string& global_name){
            if (call_back_perf.find(global_name) != call_back_perf.end()){
                return {sizeof(uint64_t), call_back_perf.find(global_name)->second};
            }
            if (call_back_perf_curbit.find(global_name) != call_back_perf_curbit.end()){
                return {sizeof(uint8_t), call_back_perf_curbit.find(global_name)->second};
            }
            assert(false);
        }

        CYCLE get_amt_l_rsim() const{return amt_lr_sim;}
        ////// sim proxy
        virtual void start_register_call_back()  = 0;
        ///// volatile and non volatile (paritial)
        virtual void start_main_op_ele_sim_neg()   = 0;
        virtual void start_main_op_ele_sim_pos()   = 0;
        ///// non-volatile
        virtual void start_finalize_ele_sim_neg() = 0;
        virtual void start_finalize_ele_sim_pos() = 0;
        ///// vcd collector
        virtual void start_vcd_dec_var_user()     = 0;
        virtual void start_vcd_dec_var_internal() = 0;
        virtual void start_vcd_col_user()        = 0;
        virtual void start_vcd_col_internal()    = 0;
        ///// start flowblock CollectData
        virtual void start_perf_col()           = 0;

        /////////////////////////////////////////
        ////// start sim optimize ///////////////
        /////////////////////////////////////////

        virtual CYCLE main_sim() = 0;

    };



}

#endif //MODEL_COMPILE_PROXY_EVENT_BASE_H
