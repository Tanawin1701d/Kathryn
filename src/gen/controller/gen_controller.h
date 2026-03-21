//
// Created by tanawin on 20/6/2024.
//

#ifndef GENCONTROLLER_H
#define GENCONTROLLER_H
#include "params/prj_param.h"
#include "front_end/cmd/param_reader.h"
#include "model/hw_component/module/module.h"
#include "abstract/main_controlable.h"
#include "util/file_writer/file_writer_base.h"
#include "gen_structure.h"
#include "util/file_writer/file_writer_group.h"
#include "util/str/str_util.h"

namespace kathryn{




    struct ModuleChecker;
    class GenController: public MainControlable{
    protected:
        const std::string PROJECT_PATH          = "..";
        const std::string path_to_synthesis_runner = PROJECT_PATH + "/synthesis_runner";
        const std::string path_to_vivado_launch    = path_to_synthesis_runner + "/launch_vivado.sh";

        const std::string _desVerilogFolderParamPrefix      = "gen_folder";
        const std::string _desVerilogTopFileNameParamPrefix = "top_file_name";
        const std::string _desVerilogTopModNameParamPrefix  = "top_mod_name";
        const std::string _desVerilogExtractParamPrefix     = "extract_mul_file";
        const std::string _desSynthesisPrefix = "syn_name";
        ////// get from parameter file
        const std::string _file_suffix = ".v";
        std::string       _desVerilogFolder;
        std::string       _desVerilogTopFileName;
        std::string       _desVerilogTopModName;
        bool              _extractMulFile = false;
        std::string       _desSynName;
        /////// working element
        FileWriterGroup   _writerGroup;
        Module*           _masterModule    = nullptr;
        ModuleGen*        _masterModuleGen = nullptr;




        void init_glob_ele(bool is_input);

    public:

        explicit GenController();

        virtual ~GenController() = default;

        /** generation have 5 major steps
         * 1. init_ele() recruit model and create its necessary meta-data
         * 2. route_io() route the io that to the collect place
         *  ///// abandon we do not maintain this anymore 3. gen_cef_all() gen meta data for compare to reduce data redundancy
         * 4. recruit_mod_to_gen_system() start compare model to reduce redundancy
         * 5. generate_every_module() dump it to verilog
         */
        void start() override;

        void init_env(PARAM& param);

        void init_ele();

        void route_io();

        ////// according to generation structure

        void generate_every_module();


        /////// this is optional /////////////
        void start_synthesis();
        /////////////////////////////////////

        void reset() override;

        void clean() override;

        //////// module compare checker
    };



    GenController*get_gen_controller_ptr();

}

#endif //GENCONTROLLER_H
