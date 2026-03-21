//
// Created by tanawin on 1/6/2024.
//

#ifndef PROXYBUILDMNG_H
#define PROXYBUILDMNG_H

#include "filesystem"
#include "string"
#include "cstdlib"
#include "utility"
#include "params/prj_param.h"
#include "front_end/cmd/param_reader.h"

#include "proxy_event_base.h"
#include "model/hw_component/module/module.h"

#include "sim/model_sim_engine/flow_block/flow_base_sim.h"
#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"
#include "sim/model_sim_engine/hw_component/module/module_sim.h"
#include "util/file_writer/file_writer_base.h"
#include "model_proxy.h"
#include "trace_event.h"
#include "user_def_repo.h"
#include "proxy_build_mode.h"


namespace kathryn{



    class ProxyBuildMng{
    protected:
        Module*          _startModule    = nullptr;
        ModuleSimEngine* module_sim_engine = nullptr;
        FileWriterBase* proxyfile_writer  = nullptr;
        FileReaderBase* proxyfile_reader  = nullptr;
        void*           _handle          = nullptr;
        UserDefRepo     _codeRepo; /// used to store old code

        const std::string TEST_NAME;
        const std::string INLINE_ATTR = "__attribute__((always_inline)) inline";
        const int         OP_LEVEL    = 3;
        const std::string OP_FLAG     = "-O3";

        //////// key of file and directory
        const std::string PROJECT_DIR    = "..";
        const std::string MD_COMPILE_FOLDER = "model_compile";

        const std::string gen_folder      = "generated";
        const std::string dyn_obj_folder   = "build";

        const std::string builder_name    = "start_gen.sh";

        ////// CODE_MAKER_TYPE match with track_keys
        enum CODE_MAKER_TYPE{CMT_INCLUDE = 0, CMT_GLOBVAR = 1, CMT_MANUALDES = 2};
        const std::string track_keys[3] = {"include", "global_var", "manual_designer"};




        //////// pathe of file and directory
        const std::string path_to_model_folder = PROJECT_DIR + "/" + MD_COMPILE_FOLDER;

        const std::string src_gen_path
        = path_to_model_folder + "/" + gen_folder + "/" + TEST_NAME + ".cpp";
        const std::string src_builder_path
        = path_to_model_folder + "/" + builder_name;
        const std::string src_dyn_load_path
        = path_to_model_folder + "/" + dyn_obj_folder + "/" + TEST_NAME +  ".so";

        /** function name*/
        ///////// [function_name][?user_ident][?ske suffix]
        const std::string BASE_CLASS_NAME = "ProxySimEvent";
        const std::string SKE_SUFFIX           = "Ske";
        const std::string USER_SUFFIX          = "User";
        const std::string INTERNAL_SUFFIX      = "Internal";
        const std::string INTERNAL_WARMUP      = "int_code_warm_up";
        const std::string REGIS_CALLBACK       = "start_register_call_back";
        const std::string REGIS_CALLBACK_LOGIC = "register_to_call_back";
        const std::string REGIS_CALLBACK_PERF  = "register_to_call_back_perf";
        const std::string CLK_MODE_POSE        = "Pos";
        const std::string CLK_MODE_NEG         = "Neg";
        const std::string MAINOP_SIM = "start_main_op_ele_sim";
        const std::string FIZOP_SIM  = "start_finalize_ele_sim";
        const std::string VCD_DEC    = "start_vcd_dec_var"; //// must have User or Internal as a suffix
        const std::string VCD_COL    = "start_vcd_col";
        const std::string PERF_COL   = "start_perf_col";
        const std::string USER_DEF   = "user_def";
        const std::string MAIN_SIM   = "main_sim";


        /** variable and value name*/
        const std::string CALLBACK_VAR_ARR_NAME    = "kathryn_call_back_meta";
        const std::string MAX_SIZE_CB_ARR          = "MAX_PROX_CALLBACK_FUNCTION";
        const std::string CALLBACK_VAR_AMT         = "kathryn_call_back_amt";
        const std::string CALLBACK_GET_AMT         = "get_call_back_amt";
        const std::string CALLBACK_GET_NO          = "get_call_back_no";
        const std::string CALLBACK_CHECK_FUNC_NAME = "check_call_back";
        const std::string VCD_WRITER_TYPE          = "VcdWriter";
        const std::string VCD_WRITER_VAR_INT       = "_vcdWriterInternal";
        const std::string VCD_WRITER_VAR_BASE_CL   = "_vcdWriter";

        std::vector<TraceEvent>*  call_back_events = nullptr;

    public:
        explicit ProxyBuildMng(std::string test_name,
                               bool req_inline = true,   ///  request function to  be inline
                               int  op_lev = 3):      ///  optimization level
        TEST_NAME(std::move(test_name)),
        INLINE_ATTR(req_inline ? "__attribute__((always_inline)) inline"
                              : ""),
        OP_LEVEL(op_lev),
        OP_FLAG("-O" + std::to_string(op_lev)),
        PROJECT_DIR(KATHRYN_PROJECT_DIR){

            mf_assert(op_lev >= 0 && op_lev <= 3,
                "invalid optimization level");

        }
        ~ProxyBuildMng();
        std::vector<ModelProxyBuild*>
        do_topology_sort(std::vector<ModelProxyBuild*>& graph);

        std::string gen_function_dec(bool class_ref, const std::string& func_name, const std::string& ret_type = "void");
        std::string gen_dummy_function_full_dec(bool class_ref, const std::string& func_name);

        void set_start_module(Module* start_module);
        void set_tracer(std::vector<TraceEvent>* tracers);

        ////////// start recruit old system
        void start_read_old_model_sim();
        ////////// generate path
        void start_write_model_sim();
        ///////// for wrate call back meta
        void start_write_call_back_var();
        ////////// for create all variable
        void start_write_create_variable();
        ///////// void start write perf create
        void start_write_perf_dec();
        ////////// start write internal warmup
        void start_write_init_internal_warm_up();
        ////////// for start register function
        void start_write_register_callback();
        ////////// call back function
        void start_write_call_back_var_init();
        void start_write_call_back_check_and_ret();
        void start_write_call_back_get_amt();
        void start_write_callback_get_no();
        ///////// for create vcd Decvar
        void start_write_vcd_dec_writer();
        void start_write_vcd_dec_var(bool is_user); //// else if internal
        ///////// for create vcd Decvar
        void start_write_vcd_col(bool is_user);
        void start_write_vcd_col_ske(bool is_user);

        ///////// void start write perf col
        void start_write_perf_col_ske();
        void start_write_perf_col();
        ///// old system
        // void start_main_op_ele_sim_ske();
        // void start_main_op_ele_sim   ();

        void start_write_all_logic_sim(CLOCK_MODE clk_mode);

        ////////// for wire expression mem_elehodler*   etc....
        ///// volatile
        void start_write_main_ele_sim_ske  (CLOCK_MODE clk_mode);
        void start_write_main_ele_sim     (CLOCK_MODE clk_mode);
        ///// non-volatile
        void start_finalize_ele_sim_ske(CLOCK_MODE clk_mode);
        void start_finalize_ele_sim   (CLOCK_MODE clk_mode);
        ////////// for user define
        void start_write_user_defined_function();
        //////// void start write for optimization
        void start_write_main_sim_ske(bool user_vcd_col,
                                  bool sys_vcd_col,
                                  bool perf_col);

        void start_write_main_sim();
        //////// void start write creator
        void start_write_create_func();

        //////// for screening clock mode
        std::vector<ModelProxyBuild*> screen_clock_mode(CLOCK_MODE clk_mode,
                                                      std::vector<ModelProxyBuild*> srcs);
        std::string get_clock_mode_str(CLOCK_MODE clk_mode);

        //////// compile file
        void start_compile();
        ///////// load path
        ProxySimEventBase* load_and_get_proxy();
        ///////// start retrieve data back
        void start_retrieve_sim_val(ProxySimEventBase* sim_event);
        ///////// disload
        void unload_proxy();

    };




}

#endif //PROXYBUILDMNG_H
