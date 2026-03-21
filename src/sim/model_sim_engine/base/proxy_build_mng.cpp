//
// Created by tanawin on 1/6/2024.
//

#include "proxy_build_mng.h"

#include "cstdlib"
#include "dlfcn.h"
#include "set"
#include "params/sim_param.h"
#include "util/term_color/term_color.h"


namespace kathryn{
    ProxyBuildMng::~ProxyBuildMng(){
        unload_proxy();
    }

    std::vector<ModelProxyBuild*> ProxyBuildMng::do_topology_sort(
        std::vector<ModelProxyBuild*>& graph){
        if (graph.empty()){ return {}; }

        struct MPD_META{
            ModelProxyBuild* mpd = nullptr;
            int next_dep_idx = 0;
        };

        /** initialize variable*/
        std::set<ModelProxyBuild*> visited;
        std::set<ModelProxyBuild*> resulted;
        std::set<ModelProxyBuild*> in_graph; ////// somedep is come from not related sorce
        std::vector<ModelProxyBuild*> result;
        std::stack<MPD_META> dfs;
        ///////////////////////////////////////////////
        for (ModelProxyBuild* mdp : graph){ in_graph.insert(mdp); }


        for (auto ele : graph){
            assert(dfs.empty());
            dfs.push({ele, 0});

            while (!dfs.empty()){
                MPD_META& top = dfs.top();

                if (in_graph.find(top.mpd) == in_graph.end()){
                    dfs.pop(); ///// not in region of interest
                    continue;
                }

                bool is_visit = visited.find(top.mpd) != visited.end();
                bool is_result = resulted.find(top.mpd) != resulted.end();


                if (top.next_dep_idx == 0){
                    ////////// first in in stack
                    if (is_visit){
                        mf_assert(is_result,
                                 "cycle dep detect at node " + top.mpd->get_val_rep().get_data());
                        dfs.pop();
                        continue;
                    }
                    visited.insert(top.mpd);
                }


                if (top.next_dep_idx == top.mpd->get_dep().size()){
                    //////////// last in stack
                    if (!is_result){
                        resulted.insert(top.mpd);
                        result.push_back(top.mpd);
                    }
                    dfs.pop();
                }
                else{
                    /////// in search
                    ModelProxyBuild* next_ele = top.mpd->get_dep()[top.next_dep_idx];
                    dfs.push({next_ele, 0});
                    top.next_dep_idx++;
                }
            }
        }
        return result;
    }

    std::string ProxyBuildMng::gen_function_dec(bool class_ref,
                                              const std::string& func_name,
                                              const std::string& ret_type){
        return ret_type + " " + (class_ref ? (BASE_CLASS_NAME + "::"): "") + func_name + "()";
    }

    std::string ProxyBuildMng::gen_dummy_function_full_dec(bool class_ref,
                                              const std::string& func_name){

        return gen_function_dec(class_ref, func_name) +  "{}\n";

    }


    void ProxyBuildMng::set_start_module(Module* start_module){
        _startModule = start_module;
        module_sim_engine = start_module->get_sim_engine_ptr();
    }
    void ProxyBuildMng::set_tracer(std::vector<TraceEvent>* tracers){
        assert(tracers != nullptr);
        call_back_events = tracers;
    }

    void ProxyBuildMng::start_read_old_model_sim(){

        for (const std::string& track_key : track_keys){
            _codeRepo.add_track_key(track_key);
        }

        if (!std::filesystem::exists(src_gen_path)){
            return;
        }

        proxyfile_reader = new FileReaderBase(src_gen_path);
        _codeRepo.fetch_code(proxyfile_reader);
        delete proxyfile_reader;

    }


    void ProxyBuildMng::start_write_model_sim(){

        bool user_vcd = (PARAM_VCD_REC_POL == MDE_REC_BOTH) ||
                       (PARAM_VCD_REC_POL == MDE_REC_ONLY_USER);

        bool internal_vcd = (PARAM_VCD_REC_POL == MDE_REC_BOTH) ||
                           (PARAM_VCD_REC_POL == MDE_REC_ONLY_INTERNAL);

        bool perf_col      = PARAM_PERF_REC_POL == MFP_ON;

        assert(module_sim_engine != nullptr);

        module_sim_engine->proxy_build_init();
        /** create file */
        proxyfile_writer = new FileWriterBase(src_gen_path);
        /** write include*/
        proxyfile_writer->add_data("#include \"../proxy_event.h\"\n");
        /** write designer manual include*/
        proxyfile_writer->add_data(_codeRepo.gen_code(track_keys[CMT_INCLUDE])+"\n"); //// include
        /** create namespace*/
        proxyfile_writer->add_data("namespace kathryn{\n\n\n\n\n\n");

        ///////// global variable
        start_write_call_back_var_init(); /// write variable for callback variable
        start_write_vcd_dec_writer();    /// write vcd writer variable
        start_write_create_variable();  /// create variable for simulation model
        start_write_perf_dec();         /// create variable for performance
        proxyfile_writer->add_data(_codeRepo.gen_code(track_keys[CMT_GLOBVAR])+"\n"); /// globalvar
        //////////////////////////////
        ///////// start  Writefunction
        //////////////////////////////
        /// register callback
        start_write_init_internal_warm_up(); //// warm-up vcd writer to bring from proxysim event
        start_write_register_callback();   //// register the performance and simvalue to make userland simulatable
        //// callback_mng
        start_write_call_back_check_and_ret(); //// callback checker
        start_write_call_back_get_amt();      //// get amount of call back
        start_write_callback_get_no();
        /// vcd DecVar collect
        if (user_vcd){
            start_write_vcd_dec_var (true);
            start_write_vcd_col_ske (true);
            start_write_vcd_col    (true);
        }else{
            ///// no need to build ske
            proxyfile_writer->add_data(gen_dummy_function_full_dec(true, VCD_DEC + USER_SUFFIX));
            proxyfile_writer->add_data(gen_dummy_function_full_dec(true, VCD_COL + USER_SUFFIX));
        }
        if (internal_vcd){
            start_write_vcd_dec_var (false);
            start_write_vcd_col_ske (false);
            start_write_vcd_col    (false);
        }else{
            ///// no need to build ske
            proxyfile_writer->add_data(gen_dummy_function_full_dec(true, VCD_DEC + INTERNAL_SUFFIX));
            proxyfile_writer->add_data(gen_dummy_function_full_dec(true, VCD_COL + INTERNAL_SUFFIX));
        }
        /// perfcollection
        if (perf_col){
            start_write_perf_col_ske();
            start_write_perf_col();
        }else{
            proxyfile_writer->add_data(gen_dummy_function_full_dec(true, PERF_COL));
        }

        ///// write the neg edge function
        start_write_all_logic_sim(CM_NEGEDGE);
        ///// write the pos edge function
        start_write_all_logic_sim(CM_POSEDGE);


        // user sim op
        start_write_user_defined_function();
        /// main sim
        start_write_main_sim_ske(user_vcd, internal_vcd, perf_col);
        start_write_main_sim();
        start_write_create_func();
        /// w flush
        proxyfile_writer->add_data("\n\n\n\n\n\n}\n");
        proxyfile_writer->flush();
        delete proxyfile_writer;
    }

    void ProxyBuildMng::start_write_call_back_var(){
        proxyfile_writer->add_data(CALLBACK_VAR_ARR_NAME + "["+ MAX_SIZE_CB_ARR+"];\n");
        proxyfile_writer->add_data(CALLBACK_VAR_AMT +";\n");
    }

    void ProxyBuildMng::start_write_create_variable(){
        assert(module_sim_engine != nullptr);
        /////// recruit model_builder for create variable
        std::vector<ModelProxyBuild*> dayta =
            module_sim_engine->recruit_for_create_var();
        /////// generate the data
        CbBaseCxx cb;
        for (ModelProxyBuild* mpb : dayta){
            mpb->create_global_variable(cb);
        }
        ////////// write file
        proxyfile_writer->add_data(cb.to_string(0));
    }


    void ProxyBuildMng::start_write_perf_dec(){
        std::vector<FlowBaseSimEngine*> dayta =
            module_sim_engine->recruit_perf();

        proxyfile_writer->add_data("/////////////////////// perf variable\n");

        CbBaseCxx cb;
        for (ModelProxyBuild* mpb : dayta){
            mpb->create_global_variable(cb);
        }
        proxyfile_writer->add_data(cb.to_string(0));
        proxyfile_writer->add_data("/////////////////////// perf finish initialize\n");
    }

    void ProxyBuildMng::start_write_init_internal_warm_up(){

        proxyfile_writer->add_data(gen_function_dec(true, INTERNAL_WARMUP)+ "{\n");
        proxyfile_writer->add_data(VCD_WRITER_VAR_INT + "=" + VCD_WRITER_VAR_BASE_CL  + ";\n");
        proxyfile_writer->add_data("}\n");

    }

    void ProxyBuildMng::start_write_register_callback(){


        auto register_helper = [&](std::string var_name, bool is_perf){
            assert(!var_name.empty());
            proxyfile_writer->add_data("       ");
            if (is_perf){
                proxyfile_writer->add_data(REGIS_CALLBACK_PERF + "(");
            }
            else{
                proxyfile_writer->add_data(REGIS_CALLBACK_LOGIC + "(");
            }
            proxyfile_writer->add_data("\"" + var_name + "\"");
            proxyfile_writer->add_data(",");
            proxyfile_writer->add_data(var_name);
            proxyfile_writer->add_data(");\n");
        };

        proxyfile_writer->add_data(gen_function_dec(true, REGIS_CALLBACK)+ "{\n");

        ///// for logic value
        for (ModelProxyBuild* mpb :
             module_sim_engine->recruit_for_regis_var()){
            register_helper(mpb->get_val_rep().get_data(), false);
        }
        ///// for perf value     we are sure that name will not
        for (FlowBaseSimEngine* mpb : module_sim_engine->recruit_perf()){
            std::vector<std::string> result;
            mpb->get_recur_var_name(result);
            mpb->get_recur_var_name_cur_stsatus(result);
            for (const std::string& var_name : result){
                register_helper(var_name, true);
            }
        }

        proxyfile_writer->add_data("}\n");
    }

    void ProxyBuildMng::start_write_call_back_var_init(){
        proxyfile_writer->add_data("int " + CALLBACK_VAR_AMT +"=0;\n");
        proxyfile_writer->add_data("int " + CALLBACK_VAR_ARR_NAME +"["+
            MAX_SIZE_CB_ARR + "];\n");
    }

    void ProxyBuildMng::start_write_call_back_check_and_ret(){

        proxyfile_writer->add_data(INLINE_ATTR + " bool " +
            CALLBACK_CHECK_FUNC_NAME + "(){\n");
        proxyfile_writer->add_data("bool should_ret = false;\n");

        for (int i = 0; i < call_back_events->size(); i++ ){
            std::string condition = (*call_back_events)[i].get_cond_str();
            CbIfCxx x(false, condition);
            x.add_st("should_ret = true");
            x.add_st(CALLBACK_VAR_ARR_NAME + "[" + CALLBACK_VAR_AMT + "] = " + std::to_string(i) );
            x.add_st(CALLBACK_VAR_AMT + "++");
            proxyfile_writer->add_data(x.to_string(0));
        }


        proxyfile_writer->add_data("return should_ret;\n");
        proxyfile_writer->add_data("}\n");

    }

    void ProxyBuildMng::start_write_call_back_get_amt(){
        proxyfile_writer->add_data("int " + BASE_CLASS_NAME + "::" + CALLBACK_GET_AMT + "() const");
        proxyfile_writer->add_data("{\n");
        proxyfile_writer->add_data("return " + CALLBACK_VAR_AMT + ";\n");
        proxyfile_writer->add_data("}\n");
    }

    void ProxyBuildMng::start_write_callback_get_no(){
        proxyfile_writer->add_data("int " + BASE_CLASS_NAME + "::" + CALLBACK_GET_NO + "(int idx) const");
        proxyfile_writer->add_data("{\n");
        proxyfile_writer->add_data("return " + CALLBACK_VAR_ARR_NAME +"[idx];\n");
        proxyfile_writer->add_data("}\n");
    }

    void ProxyBuildMng::start_write_vcd_dec_writer(){
        proxyfile_writer->add_data(VCD_WRITER_TYPE + "* " +
                                 VCD_WRITER_VAR_INT + "= nullptr;\n");
    }

    void ProxyBuildMng::start_write_vcd_dec_var(bool is_user){
        std::vector<LogicSimEngine*> dayta =
            module_sim_engine->recruit_for_vcd_var();

        std::string f_suffix = is_user ? USER_SUFFIX: INTERNAL_SUFFIX;
        proxyfile_writer->add_data(gen_function_dec(true, VCD_DEC + f_suffix));
        proxyfile_writer->add_data("{\n");


        for (LogicSimEngine* mpb : dayta){
            if (mpb->is_user_declare() == is_user){
                ////// the registerable must always put to vcd file
                proxyfile_writer->add_data("       ");
                proxyfile_writer->add_data("_vcdWriter->add_new_var(");

                //////// sigtype
                VCD_SIG_TYPE vst = mpb->get_sig_type();
                if (vst == VST_REG){
                    proxyfile_writer->add_data("VST_REG");
                }
                else if (vst == VST_WIRE){
                    proxyfile_writer->add_data("VST_WIRE");
                }
                else if (vst == VST_INTEGER){
                    proxyfile_writer->add_data("VST_INTEGER");
                }
                else{
                    assert(false);
                }

                /////// varname
                proxyfile_writer->add_data(",");
                proxyfile_writer->add_data("\"" + mpb->get_val_rep().get_data() + "\"");
                proxyfile_writer->add_data(",");
                proxyfile_writer->add_data("{" +
                    std::to_string(mpb->get_size().start) + "," +
                    std::to_string(mpb->get_size().stop) + "});\n");
            }
        }
        proxyfile_writer->add_data("}\n");
    }

    void ProxyBuildMng::start_write_vcd_col_ske(bool is_user){
        std::vector<LogicSimEngine*> dayta =
            module_sim_engine->recruit_for_vcd_var();

        std::string f_suffix = is_user ? USER_SUFFIX: INTERNAL_SUFFIX;
        proxyfile_writer->add_data(INLINE_ATTR + " " +
            gen_function_dec(false, VCD_COL + f_suffix + SKE_SUFFIX));
        proxyfile_writer->add_data("{\n");
        //proxyfile_writer->add_data("}\n");
        //return;

        for (LogicSimEngine* mpb : dayta){
            if (mpb->is_user_declare() == is_user){
                ////// the registerable must always put to vcd file
                proxyfile_writer->add_data("       ");
                proxyfile_writer->add_data(VCD_WRITER_VAR_INT + "->add_new_value(");

                proxyfile_writer->add_data("\"" + mpb->get_val_rep().get_data() + "\"");
                proxyfile_writer->add_data(",");
                if (mpb->getValR_Type().type == SVT_U64M){
                    //proxyfile_writer->add_data("0");
                    proxyfile_writer->add_data(mpb->get_val_rep().get_data() + ".to_bi_str()");
                }else{
                    proxyfile_writer->add_data(mpb->get_val_rep().get_data());
                }
                proxyfile_writer->add_data(");\n");
            }
        }

        proxyfile_writer->add_data("}\n");
    }

    void ProxyBuildMng::start_write_vcd_col(bool is_user){

        std::string f_suffix = is_user ? USER_SUFFIX: INTERNAL_SUFFIX;
        proxyfile_writer->add_data(gen_function_dec(true, VCD_COL + f_suffix ));

        proxyfile_writer->add_data("{\n");
        proxyfile_writer->add_data("      " + VCD_COL + f_suffix + SKE_SUFFIX);
        proxyfile_writer->add_data("();\n");
        proxyfile_writer->add_data("}\n");

    }

    void ProxyBuildMng::start_write_perf_col_ske(){
        std::vector<FlowBaseSimEngine*> dayta = module_sim_engine->recruit_perf();

        proxyfile_writer->add_data(INLINE_ATTR + " " +
            gen_function_dec(false, PERF_COL+  SKE_SUFFIX));
        proxyfile_writer->add_data("{\n");

        CbBaseCxx cb;
        for (ModelProxyBuild* mpb : dayta){
            mpb->create_op(cb);
        }
        proxyfile_writer->add_data(cb.to_string(0));
        proxyfile_writer->add_data("}\n");
    }


    void ProxyBuildMng::start_write_perf_col(){
        proxyfile_writer->add_data(gen_function_dec(true, PERF_COL) + "{\n");
        proxyfile_writer->add_data("      start_perf_col_ske();\n");
        proxyfile_writer->add_data("}\n");

    }

    void ProxyBuildMng::start_write_all_logic_sim(CLOCK_MODE clk_mode){
        start_write_main_ele_sim_ske  (clk_mode);
        start_write_main_ele_sim     (clk_mode);
        ///// non-volatile
        start_finalize_ele_sim_ske(clk_mode);
        start_finalize_ele_sim   (clk_mode);
    }


    //////// non volatile must do TOPOLOGY SORT FIRST

    void ProxyBuildMng::start_write_main_ele_sim_ske(CLOCK_MODE clk_mode){
        /***
         *
         * 1.standard op
         * 2.mem_assign memnt
         * 3.transfer op
         *
         */
        ///////////// do not worry about register simulation will get false new
        ///data from memory if there is update from memory to register because
        /// mem_ele_holder will provide temporary data to register simulation

        std::vector<ModelProxyBuild*> volatile_ele    = module_sim_engine->recruit_for_main_op_volatile();
        std::vector<ModelProxyBuild*> non_volatile_ele = screen_clock_mode(clk_mode,
                                                                       module_sim_engine->recruit_for_main_op_non_volatile());

        proxyfile_writer->add_data(INLINE_ATTR + " " +
                                 gen_function_dec(false, MAINOP_SIM + get_clock_mode_str(clk_mode)  + SKE_SUFFIX) +
                                 "{\n");

        //////// optimize to zero out the negative edge workload
        if ((clk_mode == CM_NEGEDGE) && non_volatile_ele.empty()){ /////// there is no
            proxyfile_writer->add_data("}\n");
            return;
        }

        //////////////////////// create local variable
        CbBaseCxx cb_init_val;
        for (ModelProxyBuild* mpb : volatile_ele){
            mpb->create_local_variable(cb_init_val);
        }
        for (ModelProxyBuild* mpb : non_volatile_ele){
            mpb->create_local_variable(cb_init_val);
        }
        proxyfile_writer->add_data(cb_init_val.to_string(0));
        //////////////////////// volatile sim
        proxyfile_writer->add_data("//// do main sim volatile\n");
        CbBaseCxx cb_volatile_sim;
        std::vector<ModelProxyBuild*> sorted_volatile_ele = do_topology_sort(volatile_ele);
        for (ModelProxyBuild* mpb : sorted_volatile_ele){
            mpb->create_op(cb_volatile_sim);
            //break;

        }
        proxyfile_writer->add_data(cb_volatile_sim.to_string(0));
        //////////////////////// non_volatile sim
        proxyfile_writer->add_data("//// do main sim NON volatile\n");
        CbBaseCxx cb_non_volatile_sim;
        for (ModelProxyBuild* mpb : non_volatile_ele){
            /////// don't have to sort
            mpb->create_op(cb_non_volatile_sim);
        }
        proxyfile_writer->add_data(cb_non_volatile_sim.to_string(0));

        proxyfile_writer->add_data("}\n");
    }

    void ProxyBuildMng::start_write_main_ele_sim(CLOCK_MODE clk_mode){
        proxyfile_writer->add_data(gen_function_dec(true, MAINOP_SIM + get_clock_mode_str(clk_mode)) + "{\n");
        proxyfile_writer->add_data(MAINOP_SIM + get_clock_mode_str(clk_mode) + SKE_SUFFIX + "();\n");
        proxyfile_writer->add_data("}\n");
    }

    void ProxyBuildMng::start_finalize_ele_sim_ske(CLOCK_MODE clk_mode){
        /***
         *
         * 1.standard op
         * 2.mem_assign memnt
         * 3.transfer op
         *
         */

        ///////////// do not worry about register simulation will get false new
        ///data from memory if there is update from memory to register because
        /// mem_ele_holder will provide temporary data to register simulation

        proxyfile_writer->add_data(INLINE_ATTR + " "+
                                 gen_function_dec(false, FIZOP_SIM +
                                                       get_clock_mode_str(clk_mode) +
                                                       SKE_SUFFIX) +
                                 "{\n");

        std::vector<ModelProxyBuild*> mpbs = screen_clock_mode(clk_mode,
                                                             module_sim_engine->recruit_for_finalize_op());

        if ((clk_mode == CM_NEGEDGE) && mpbs.empty()){ /////// there is no
            proxyfile_writer->add_data("}\n");
            return;
        }

        //////////////////// priority 1 ///////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////
        CbBaseCxx cb_final1; ///// typically mem_op
        proxyfile_writer->add_data("////////////////////// transfer  op priority 1\n");
        for (ModelProxyBuild* mpb : mpbs){
            mpb->create_op_end_cycle(cb_final1);
        }
        proxyfile_writer->add_data(cb_final1.to_string(0));
        //////////////////// priority 2 ///////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////
        CbBaseCxx cb_final2; ///// typically mem_op
        proxyfile_writer->add_data("//////////////////// transfer Op priority 2\n");
        for (ModelProxyBuild* mpb : mpbs){
            mpb->create_op_end_cycle2(cb_final2);

        }
        proxyfile_writer->add_data(cb_final2.to_string(0));

        proxyfile_writer->add_data("}\n");
    }

    void ProxyBuildMng::start_finalize_ele_sim(CLOCK_MODE clk_mode){
        proxyfile_writer->add_data(gen_function_dec(true, FIZOP_SIM + get_clock_mode_str(clk_mode)) + "{\n");
        proxyfile_writer->add_data(FIZOP_SIM + get_clock_mode_str(clk_mode) + SKE_SUFFIX +"();\n");
        proxyfile_writer->add_data("}\n");
    }

    void ProxyBuildMng::start_write_user_defined_function(){

        proxyfile_writer->add_data(INLINE_ATTR + " "+
                                 gen_function_dec(false, USER_DEF + USER_SUFFIX + SKE_SUFFIX)+
                                 "{\n");
        proxyfile_writer->add_data("\n\n\n\n\n\n\n\n\n\n\n");
        assert(module_sim_engine != nullptr);
        /////// recruit model_builder for create user mark value
        std::vector<ModelProxyBuild*> dayta =
            module_sim_engine->recruit_for_create_var();
        /////// generate the data
        CbBaseCxx cb;
        for (ModelProxyBuild* mpb : dayta){
            mpb->create_user_mark_value(cb);
        }
        ////////// write the designer
        proxyfile_writer->add_data(cb.to_string(0));
        proxyfile_writer->add_data(_codeRepo.gen_code(track_keys[CMT_MANUALDES])+"\n"); /// manual_designer
        proxyfile_writer->add_data("}\n");

    }


    void ProxyBuildMng::start_write_main_sim_ske(bool user_vcd_col,
                                             bool sys_vcd_col,
                                             bool perf_col){
        proxyfile_writer->add_data("CYCLE " + MAIN_SIM + SKE_SUFFIX + "(CYCLE kathryn_longrangeLim){\n");
        proxyfile_writer->add_data("CYCLE kathryn_longrangeCnt  = 0;\n");
        proxyfile_writer->add_data("do{\n");
        /////// TODO add tricker Event
        proxyfile_writer->add_data(USER_DEF   + USER_SUFFIX + SKE_SUFFIX + "();\n");
        proxyfile_writer->add_data("///// do negative edge\n");
        proxyfile_writer->add_data(MAINOP_SIM + get_clock_mode_str(CM_NEGEDGE)   + SKE_SUFFIX + "();\n");
        if (user_vcd_col){ proxyfile_writer->add_data(VCD_COL + USER_SUFFIX     + SKE_SUFFIX + "();\n");}
        if (sys_vcd_col) { proxyfile_writer->add_data(VCD_COL + INTERNAL_SUFFIX + SKE_SUFFIX + "();\n");}
        proxyfile_writer->add_data(FIZOP_SIM + get_clock_mode_str(CM_NEGEDGE)    + SKE_SUFFIX + "();\n");

        proxyfile_writer->add_data("///// do positive edge\n");
        proxyfile_writer->add_data(MAINOP_SIM + get_clock_mode_str(CM_POSEDGE)   + SKE_SUFFIX + "();\n");
        if (user_vcd_col){ proxyfile_writer->add_data(VCD_COL + USER_SUFFIX     + SKE_SUFFIX + "();\n");}
        if (sys_vcd_col) { proxyfile_writer->add_data(VCD_COL + INTERNAL_SUFFIX + SKE_SUFFIX + "();\n");}
        proxyfile_writer->add_data(FIZOP_SIM + get_clock_mode_str(CM_POSEDGE)    + SKE_SUFFIX + "();\n");

        if (perf_col){ proxyfile_writer->add_data(PERF_COL + SKE_SUFFIX + "();\n");}
        proxyfile_writer->add_data("kathryn_longrangeCnt++;\n");
        proxyfile_writer->add_data("}"); //// out of while loop
        proxyfile_writer->add_data("while(!" + CALLBACK_CHECK_FUNC_NAME + "() && "
                                "(kathryn_longrangeCnt < kathryn_longrangeLim)"
                                                                        ");\n");
        proxyfile_writer->add_data("return kathryn_longrangeCnt;\n");
        proxyfile_writer->add_data("}\n");
    }

    void ProxyBuildMng::start_write_main_sim(){
        proxyfile_writer->add_data("CYCLE " + BASE_CLASS_NAME + "::"+ MAIN_SIM + "(){\n");
        proxyfile_writer->add_data(CALLBACK_VAR_AMT + " = 0;\n");
        proxyfile_writer->add_data( "return " + MAIN_SIM + SKE_SUFFIX + "(_amtLimitLongRangeCycle);\n");
        proxyfile_writer->add_data("}\n");
    }

    void ProxyBuildMng::start_write_create_func(){
        proxyfile_writer->add_data(
            "extern \"C\" ProxySimEventBase* create() {\n"
#ifdef MODELCOMPILEVB
        "   std::cout << \"creating proxy sim_event in dynamic object\" << std::endl;\n"
#endif
            "   return new ProxySimEvent();\n}\n\n"
        );
    }

    std::vector<ModelProxyBuild*> ProxyBuildMng::screen_clock_mode(
        CLOCK_MODE clk_mode,
        std::vector<ModelProxyBuild*> srcs){

        std::vector<ModelProxyBuild*> result;
        for (ModelProxyBuild* src: srcs){
            if (src->get_clock_mode() ==  CM_CLK_UNUSED ||
                src->get_clock_mode() ==  CM_AMT){
                std::cout << TC_YELLOW
                          << src->get_val_rep().get_data()
                          << " has no CLOCK sensitivity list "
                          << TC_DEF << std::endl;
            }
            if (src->get_clock_mode() == clk_mode){
                result.push_back(src);
            }
        }
        return result;
    }

    std::string ProxyBuildMng::get_clock_mode_str(CLOCK_MODE clk_mode){
        switch (clk_mode){
            case CM_POSEDGE: return CLK_MODE_POSE;
            case CM_NEGEDGE: return CLK_MODE_NEG;
            default: assert(false); return "";
        }
    }



    void ProxyBuildMng::start_compile(){
        std::string compile_comand =
            src_builder_path + " " +
            TEST_NAME + " " +
            PROJECT_DIR + " " +
            OP_FLAG;

#ifdef MODELCOMPILEVB
            std::cout << "compile command is " << compile_comand << std::endl;
#endif
        int result = system(compile_comand.c_str());
#ifdef MODELCOMPILEVB
        if (result == 0){
            std::cout << "compile successfully\n";
        }else{
            std::cerr << "Compilation failed with error code " << result << std::endl;
        }
#endif
    }

    ProxySimEventBase* ProxyBuildMng::load_and_get_proxy(){
        //////// open dynamic link
        _handle = dlopen(src_dyn_load_path.c_str(), RTLD_LAZY);
        const char* dlopen_error = dlerror();
        if (dlopen_error){
            std::cerr << "can't dynamic open " << dlopen_error << std::endl;
            exit(EXIT_FAILURE);
        }

        /////// dynamic load symbol
        typedef ProxySimEventBase* (*SeCreator)();
        SeCreator create = (SeCreator)dlsym(_handle, "create");
        const char* dlsym_error = dlerror();
        if (dlsym_error){
            std::cerr << "can't load symbol 'create': " << dlsym_error << std::endl;
            dlclose(_handle);
            exit(EXIT_FAILURE);
        }
        /////// create object
        ProxySimEventBase* pixie_event = create();
        //pixie_event->event_warm_up();
        //pixie_event->start_vcd_dec_var_internal();
        return pixie_event;
    }

    void ProxyBuildMng::start_retrieve_sim_val(ProxySimEventBase* sim_event){
        assert(sim_event != nullptr);
        assert(_startModule != nullptr);
        module_sim_engine->retrieve_init(sim_event);
    }

    void ProxyBuildMng::unload_proxy(){
        int close_status = dlclose(_handle);
        assert(close_status == 0);
    }

}
