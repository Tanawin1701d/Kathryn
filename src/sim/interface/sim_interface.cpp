//
// Created by tanawin on 25/1/2567.
//

#include "iostream"

#include "utility"

#include "sim_interface.h"
#include "params/sim_param.h"


namespace kathryn{

    /**
     *
     * sim interface
     *
     * */



    SimInterface::SimInterface(CYCLE limit_cycle,
                               std::string vcd_file_path,
                               std::string profile_file_path,
                               std::string gen_file_name,
                               SimProxyBuildMode sim_proxy_build_mode,
                               bool require_lrc,
                               bool req_inline,
                               int  op_level
                               ):
            _simProxyBuildMode(sim_proxy_build_mode),
            _vcdWriter (new VcdWriter(std::move(vcd_file_path))),
            _flowWriter(new FlowWriter(std::move(profile_file_path))),
            _limitCycle(limit_cycle),
            _proxyBuildMng(std::move(gen_file_name), req_inline, op_level),
            _requireLRC(require_lrc)
    {
        SimController* sim_ctrl = get_sim_controller_ptr();
        assert(sim_ctrl != nullptr);
        sim_ctrl->set_limit_cycle(limit_cycle);
    }

    SimInterface::~SimInterface() {
        delete _vcdWriter;
        delete _flowWriter;
        ////delete _modelSimEvent;
        /**no need to delete user event because sim controller will delete it */
    }

    void SimInterface::sim_start() {
        /** set assiging mode*/
        set_assign_mode(AM_SIM);
        /***compile and link module sim Event */
        describe_model_trigger_wrapper();
        create_model_sim_event();
        if (!has_config(_simProxyBuildMode, SimProxyBuildMode::SPB_RUN)){
            std::cout << TC_YELLOW
            << "sim_interface stop due to SPB_RUN is not set"
            << TC_DEF << std::endl;
            return;
        }
        init_perf_col();
        /***con simulating*/
        describe_def();
        describe();
        /**con simulating*/
        if (require_con_sim){
            sim_start_con_sim();
        }
        /** start main thread*/
        SimController* sim_ctrl = get_sim_controller_ptr();
        assert(sim_ctrl != nullptr);
        sim_ctrl->set_trigger_map(&_traceEvents);
        sim_ctrl->set_lr_lim_user(&_nextLimitAmtLRC);
        sim_ctrl->start();
        /** sim ctrl now finish next terminate our sim specifier*/
        if (require_con_sim && con_thread && con_thread->joinable()){
            con_thread->join();
        }
        /** set assiging mode back*/
        set_assign_mode(AM_MOD);
        final_perf_col();
        /** finish*/

    }


    void SimInterface::describe_def(){
        sim{
            *rst_wire = 1;
        };
        inc_cycle(1);
        sim{
            *rst_wire = 0;
        };

    }



    /***
     *
     *
     * Concrete simulation
     *
     * */


    void SimInterface::sim_start_con_sim(){
        /**build new auto start trigger*/
        last_ct_trigger = new ConcreteTriggerEvent(2,this,
                                                 [&](){return true;}, SIM_CC_TRIGGER_PRIO_FRONT_CYCLE);
        con_thread = std::make_unique<std::thread>(&SimInterface::describe_con_wrapper,this);
        assert(con_thread != nullptr);
    }

    void SimInterface::con_cycle_base(CYCLE start_cycle){

        last_ct_trigger->finish_sim_cur_event.notify(con_cur_cycle_used);
        last_ct_trigger->start_end_cycle_event.wait(con_cur_cycle_used);
        last_ct_trigger->set_future_cycle(start_cycle);
        last_ct_trigger->finish_end_cycle_event.notify(con_cur_cycle_used);
        last_ct_trigger->start_sim_cur_event.wait(start_cycle);
        con_cur_cycle_used = start_cycle;


    }

    void SimInterface::con_cycle(CYCLE start_cycle){
        con_cycle_base(start_cycle);
    }

    void SimInterface::con_next_cycle(CYCLE amt_cycle) {
        con_cycle(con_cur_cycle_used + amt_cycle);
    }

    void SimInterface::con_end_cycle() {
        last_ct_trigger->finish_sim_cur_event.notify(con_cur_cycle_used);
        last_ct_trigger->start_end_cycle_event.wait(con_cur_cycle_used);
    }


    void SimInterface::describe_con_wrapper() {
        assert(last_ct_trigger != nullptr);
        last_ct_trigger->start_sim_cur_event.wait(2);
        describe_con();
        ///////std::cout << "last trigger notifying" << std::endl;
        last_ct_trigger->finish_sim_cur_event.notify(con_cur_cycle_used);
        last_ct_trigger->mark_stop();
        last_ct_trigger->finish_end_cycle_event.notify(con_cur_cycle_used);
        /** to prevent queue stuck*/
    }


    /**
     *
     * trigger manager
     *
     ***/

    void SimInterface::set_next_limit_amt_lrc(CYCLE amt_cycle){
        assert(amt_cycle > 0);
        _nextLimitAmtLRC = amt_cycle;
    }

    void SimInterface::trig(Operable& opr1, std::function<void()> callback){
        _traceEvents.emplace_back(opr1, callback);
    }

    void SimInterface::trig(Operable& opr1, SIM_INTERFACE_EVENT event){
        switch (event){
            case EXIT_SIM:{
                _traceEvents.emplace_back(opr1, [](){
                    get_sim_controller_ptr()->stop_sim();
                });
            }
            default : {assert(false);}
        }
        ///// TODO make the system exit  and

    }

    void SimInterface::describe_model_trigger_wrapper(){
        get_controller_ptr()->on_globalModule_init_auxilaryComponent();
        describe_model_trigger();
        get_controller_ptr()->on_globalModule_final_auxilaryComponent();
    }


    /***
     *
     * test value
     *
     */

    void SimInterface::test_and_print(const std::string& test_name, ValRepBase& sim_val_lhs, ValRepBase& test_val_rhs) {

        if (sim_val_lhs.get_val() == test_val_rhs.get_val()){
            std::cout << TC_GREEN << test_name << " pass " << TC_DEF << std::endl;
        }else{
            std::cout << TC_RED << test_name << " fail expect: "
                      << std::to_string(test_val_rhs.get_val()) << "  got : "
                      << std::to_string(sim_val_lhs.get_val())  << TC_DEF << std::endl;
        }
    }

    void SimInterface::test_and_print(const std::string& test_name, ull sim_val, ull expect) {

        if (sim_val == expect) {
            std::cout << TC_GREEN << test_name << " pass " << TC_DEF << std::endl;
        } else {
            std::cout << TC_RED << test_name << " fail expect: "
                      << expect << "  got : "
                      << sim_val << TC_DEF << std::endl;
        }

    }


    /**
     *
     * create model client
     *
     */

    void SimInterface::create_model_sim_event(){

        /** generate c++ file**/
        _proxyBuildMng.set_start_module(get_global_module_ptr()); /// todo , SIM_CLIENT_PATH);
        _proxyBuildMng.set_tracer(&_traceEvents);

        if (has_config(_simProxyBuildMode, SimProxyBuildMode::SPB_GEN)){
            _proxyBuildMng.start_read_old_model_sim();
            _proxyBuildMng.start_write_model_sim();
        }
        if (has_config(_simProxyBuildMode, SimProxyBuildMode::SPB_COMPILE)){
            _proxyBuildMng.start_compile();
        }
        if (!has_config(_simProxyBuildMode, SimProxyBuildMode::SPB_RUN)){
            return;
        }
        _modelSimEvent = _proxyBuildMng.load_and_get_proxy();

        ///////// initialize both simevent and proxy_build_mng
        _modelSimEvent->set_long_run_type(_requireLRC);
        _modelSimEvent->set_vcd_writer(_vcdWriter);
        _modelSimEvent->set_vcd_write_pol(PARAM_VCD_REC_POL);
        _modelSimEvent->event_warm_up();
        _modelSimEvent->int_code_warm_up();
        _proxyBuildMng.start_retrieve_sim_val(_modelSimEvent);

        ////////// add to event queue
        get_sim_controller_ptr()->add_event(_modelSimEvent);

    }

    void SimInterface::init_perf_col(){
        Module* global_md = get_global_module_ptr();
        assert(global_md != nullptr);
        assert(_flowWriter != nullptr);
        _flowWriter->init(global_md);
    }

    void SimInterface::final_perf_col(){
        assert(_flowWriter != nullptr);
        _flowWriter->start_write_data();
    }







}
