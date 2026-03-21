//
// Created by tanawin on 20/6/2024.
//

#include "gen_controller.h"
#include "model/controller/controller.h"
#include "model/hw_component/abstract/glob_pool.h"
#include "model/hw_component/wire/wire_auto.h"


namespace kathryn{


    GenController::GenController():
    PROJECT_PATH(KATHRYN_PROJECT_DIR){}

    void GenController::init_env(PARAM& param){
        _desVerilogFolder      = param[_desVerilogFolderParamPrefix     ];
        _desVerilogTopFileName = param[_desVerilogTopFileNameParamPrefix];
        _desVerilogTopModName  = param[_desVerilogTopModNameParamPrefix ];
        _extractMulFile =
            (param[_desVerilogTopModNameParamPrefix ] == "true");
        _desSynName     = param[_desSynthesisPrefix];

        _writerGroup.set_prefix_folder(_desVerilogFolder);
        _masterModule   = get_global_module_ptr();
    }

    void GenController::init_ele(){

        _masterModule->create_module_gen();
        _masterModule->set_top_module();
        _masterModuleGen = _masterModule->get_module_gen_ptr();
        _masterModuleGen->start_init_ele();
        init_glob_ele(true); ///// init input
        init_glob_ele(false); ////// init output

    }

    void GenController::start(){
        init_ele();
        route_io();
        generate_every_module();
    }

    void GenController::route_io(){
        //////// it is recursive function
        _masterModuleGen->start_route_ele();
        _masterModuleGen->finalize_route_ele();
    }

    void GenController::generate_every_module(){

        FileWriterBase* top_writer = _writerGroup.create_new_file(_desVerilogTopFileName + _file_suffix);

        _masterModuleGen->start_write_file_master(_extractMulFile, top_writer, &_writerGroup, true, _desVerilogTopModName);

    }

    void GenController::start_synthesis(){

        assert(!_desSynName.empty());
        _writerGroup.flush_all();
        if (!_extractMulFile){
            std::cout << "we did not afford synthesis runner for multiple file yet";
            assert(false);
        }
        std::string compile_comand =
            path_to_vivado_launch + " " + _desSynName + " " +
                _desVerilogFolder + "/" + _desVerilogTopFileName;
        int compile_comd = system(compile_comand.c_str());
        std::cout << "synthesis result: " << compile_comd << std::endl;
    }

    void GenController::reset(){
        _writerGroup.clean();
    }

    void GenController::clean(){
        reset();
    }

    GenController* gen_ctrl = nullptr;

    GenController*get_gen_controller_ptr(){

        /////////// gen controller
        if (gen_ctrl == nullptr){
            gen_ctrl = new GenController();
        }
        return gen_ctrl;
    }




    ///////////////////////// inside element

    void GenController::init_glob_ele(bool is_input){
        assert(_masterModule != nullptr);
        assert(_masterModuleGen != nullptr);
        std::vector<WireAuto*>& des_save_io = is_input ? _masterModuleGen->_genWires[WIRE_AUTO_GEN_GLOB_INPUT]
                                                    : _masterModuleGen->_genWires[WIRE_AUTO_GEN_GLOB_OUTPUT];
        ///////
        /////// input/output
        ///////

        /////////// get it from global pool
        for(WireMarker* src_to_be_glob_io: get_glob_pool(is_input)){
            assert(src_to_be_glob_io != nullptr);
            Operable* origin_opr = src_to_be_glob_io->get_opr_from_glob_io_ptr();
            assert(origin_opr != nullptr);
            //////// the host wire require inserting
            auto& input_io = make_opr_io_wire(
                    src_to_be_glob_io->get_glob_io_name(),
                    origin_opr->get_operable_slice().get_size(),
                    is_input ? WIRE_AUTO_GEN_GLOB_INPUT : WIRE_AUTO_GEN_GLOB_OUTPUT
                );
            input_io.build_hierarchy(_masterModule);
            input_io.create_logic_gen();
            /////// connect
            if(is_input){

                UpdateEventBasic* connect_event =
                    create_ue_helper(
                        &input_io,
                        input_io.get_operable_slice(),
                        DEFAULT_UE_PRI_MIN,
                        CM_CLK_FREE,
                        false);

                src_to_be_glob_io->get_asb_from_wire_marker_ptr()->add_update_meta(connect_event);
            }else{
                input_io.connect_to(origin_opr, false); /////// it may needs to be route
            }
            des_save_io.push_back(&input_io);
        }

    }




}
