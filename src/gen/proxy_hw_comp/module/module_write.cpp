//
// Created by tanawin on 1/7/2024.
//
#include "model/hw_component/module/module.h"

#include "gen/controller/gen_controller.h"


namespace kathryn{


    void ModuleGen::start_write_file_master(bool               require_new_file,
                                         FileWriterBase*    upper_file_writer,
                                         FileWriterGroup*   writer_group,
                                         bool               is_explicit_mod,
                                         const std::string& explicit_mod_name){

        FileWriterBase* final_file_writer = upper_file_writer;
        if (require_new_file && (!_master->is_top_module())){
            final_file_writer = writer_group->create_new_file(get_opr() + ".v");
        }
        std::string final_mod_name = get_opr();
        if (is_explicit_mod){
            final_mod_name = explicit_mod_name;
        }
        start_write_file(final_file_writer, final_mod_name);

        ////////////// write submodule
        for (ModuleGen* sub_md_gen: _subModulePool){
            sub_md_gen->start_write_file_master(require_new_file,
                                           upper_file_writer,
                                           writer_group,
                                           false,
                                           "unknown");
        }

    }


    void ModuleGen::start_write_file(FileWriterBase* file_writer,
                                   const std::string& explicit_mod_name){
        LogicGenBaseVec sub_module_output_represent;
        LogicGenBaseVec sub_module_input_represent;

        ///////////// recruit io of sub module/////////////////////////////////////
        for (ModuleGen* sub_md_gen: _subModulePool){
            /////////// output
            recruit_logic_gen_base(sub_module_output_represent,
                                sub_md_gen->_genWires[WIRE_AUTO_GEN_OUTPUT]);
            for (LogicGenBase* md_output: sub_md_gen->_wirePoolWithOutputMarker){
                assert(md_output != nullptr);
                sub_module_output_represent.push_back(md_output);
            }
            ////////// input
            recruit_logic_gen_base(sub_module_input_represent,
                                sub_md_gen->_genWires[WIRE_AUTO_GEN_INPUT]);
            for (LogicGenBase* md_input: sub_md_gen->_wirePoolWithInputMarker){
                assert(md_input != nullptr);
                sub_module_input_represent.push_back(md_input);
            }
        }
        ///////////////////////////////////////////////////////////

        /**
         *
         *   module io declaration
         *
         */
        file_writer->add_data("//////////////////////////////////////////////////////////////////////////////\n"
                            "//////////////////// MODULE DEC ////////////////////////////////////////////////\n"
                            "////////////////////////////////////////////////////////////////////////////////\n"
                            );
        file_writer->add_data("module ");
        file_writer->add_data(explicit_mod_name+"(\n");


        //////// declare input/output element
        std::vector<std::string> io_vec = get_io_dec();
        io_vec.emplace_back("input wire clk");
        write_gen_vec(io_vec, file_writer, ",\n");
        file_writer->add_data("\n);\n");

        /***                               |   wire declaration   |   wire operation   |
         * -----------------------------------------------------------------------------
         * glob/auto/user input  wire ---->           no          |         no
         * glob/auto/user output wire ---->           no          |        yes
         * submodule input wire       ---->          yes          |        yes
         * submodule output wire      ---->          yes          |         no
         * -----------------------------------------------------------------------------
         * bridge wire                ---->          yes          |        yes
         * -----------------------------------------------------------------------------
         */

        /*
         * declare variable initiation
         *
         */
        file_writer->add_data("////reg_dec_var\n");
        write_gen_vec(_regPool.get_dec_vars()                          , file_writer, "\n"); file_writer->add_data("\n////wire_dec_var\n");
        write_gen_vec(_wirePool.get_dec_vars()                         , file_writer, "\n"); file_writer->add_data("\n////_exprPool\n");
        write_gen_vec(_exprPool.get_dec_vars()                         , file_writer, "\n"); file_writer->add_data("\n////_nestPool\n");
        write_gen_vec(_nestPool.get_dec_vars()                         , file_writer, "\n"); file_writer->add_data("\n////_valPool\n");
        write_gen_vec(_valPool.get_dec_vars()                          , file_writer, "\n"); file_writer->add_data("\n////_memBlockPool\n");
        write_gen_vec(_memBlockPool.get_dec_vars()                     , file_writer, "\n"); file_writer->add_data("\n////_memBlockElePool\n");
        write_gen_vec(_memBlockElePool.get_dec_vars()                  , file_writer, "\n"); file_writer->add_data("\n////input of submodule\n");
        write_gen_vec(sub_module_input_represent.get_dec_vars()           , file_writer, "\n"); file_writer->add_data("\n////output of submodule\n");
        write_gen_vec(sub_module_output_represent.get_dec_vars()          , file_writer, "\n"); file_writer->add_data("\n////bridge_vec\n");
        write_gen_vec(_genWirePools[WIRE_AUTO_GEN_INTER].get_dec_vars(), file_writer, "\n");file_writer->add_data("\n");

        file_writer->add_data("///////////////////////////////////////////////\n"
                            "//////////////////// operation///////////////////\n"
                            "/////////////////////////////////////////////////\n"
                            );



        file_writer->add_data("\n///reg_op\n");
        write_gen_vec(_regPool.get_dec_ops()                                , file_writer, "\n"); file_writer->add_data("\n///_wirePoolOp\n");
        write_gen_vec(_wirePool.get_dec_ops()                               , file_writer, "\n"); file_writer->add_data("\n///_exprPoolOp\n");
        write_gen_vec(_exprPool.get_dec_ops()                               , file_writer, "\n"); file_writer->add_data("\n///_nestPoolOp\n");
        write_gen_vec(_nestPool.get_dec_ops()                               , file_writer, "\n"); file_writer->add_data("\n///_valPoolOp\n");
        write_gen_vec(_valPool.get_dec_ops()                                , file_writer, "\n"); file_writer->add_data("\n///_memBlockPoolOp\n");
        write_gen_vec(_memBlockPool.get_dec_ops()                           , file_writer, "\n"); file_writer->add_data("\n///_memBlockElePoolOp\n");
        write_gen_vec(_memBlockElePool.get_dec_ops()                        , file_writer, "\n"); file_writer->add_data("\n///output User VecOp\n");
        write_gen_vec(_wirePoolWithOutputMarker.get_dec_ops()               , file_writer, "\n"); file_writer->add_data("\n///output_vec_op\n");
        write_gen_vec(_genWirePools[WIRE_AUTO_GEN_OUTPUT].get_dec_ops()     , file_writer, "\n"); file_writer->add_data("\n///output_vec_op global\n");
        write_gen_vec(_genWirePools[WIRE_AUTO_GEN_GLOB_OUTPUT].get_dec_ops(), file_writer, "\n"); file_writer->add_data("\n////input of submodule\n");
        write_gen_vec(sub_module_input_represent.get_dec_ops()                 , file_writer, "\n"); file_writer->add_data("\n///bridge_vec_op\n");
        write_gen_vec(_genWirePools[WIRE_AUTO_GEN_INTER].get_dec_ops()      , file_writer, "\n");
        ////////// declare submodule connectivity
        file_writer->add_data("/// sub module declaration\n");
        for (ModuleGen* sub_md_gen: _subModulePool){
            file_writer->add_data(sub_md_gen->get_sub_module_dec(sub_md_gen));
        }
        //////// end module
        file_writer->add_data("\nendmodule\n\n");

    }



    std::vector<std::string> ModuleGen::get_io_dec(){

        std::vector<std::string> result;
        for (LogicGenBase* lgb: _wirePoolWithInputMarker                ){result.push_back(lgb->dec_io());}
        for (LogicGenBase* lgb: _wirePoolWithOutputMarker               ){result.push_back(lgb->dec_io());}
        for (LogicGenBase* lgb: _genWirePools[WIRE_AUTO_GEN_INPUT]      ){result.push_back(lgb->dec_io());}
        for (LogicGenBase* lgb: _genWirePools[WIRE_AUTO_GEN_OUTPUT]     ){result.push_back(lgb->dec_io());}
        for (LogicGenBase* lgb: _genWirePools[WIRE_AUTO_GEN_GLOB_INPUT] ){result.push_back(lgb->dec_io());}
        for (LogicGenBase* lgb: _genWirePools[WIRE_AUTO_GEN_GLOB_OUTPUT]){result.push_back(lgb->dec_io());}

        return result;
    }

    std::vector<std::string> ModuleGen::get_param_dec(){

        std::vector<std::string> result;
        for (LogicGenBase* lgb: _pmValPool){result.push_back(lgb->dec_param_val());}
        return result;

    }

    //////////////////////////// get module dec as sub
    ///
    std::string ModuleGen::get_sub_module_dec(ModuleGen* sub_md_gen){
        assert(sub_md_gen != nullptr);

        std::vector<std::string> io_strs;
        std::string result;

        /////// get the host module to generate

        ////// declare submodule
        result += sub_md_gen->get_opr() + "  "  + sub_md_gen->get_opr();
        if (!sub_md_gen->_pmValPool.empty()){
            result += sub_md_gen->get_opr() + " #( ";

            std::vector<std::string> io_param_vals;
            for (LogicGenBase* lgb: sub_md_gen->_pmValPool){
                io_param_vals.push_back(lgb->dec_op());
            }
            result += join_str(io_param_vals, ',');

            result += ") ";
        }
        result += "(\n";

        ////////////////// declare input and output
        for (const std::string& input_str : _wirePoolWithInputMarker.get_oprs()){
            io_strs.push_back(input_str);
        }
        for (const std::string& output_str: _wirePoolWithOutputMarker.get_oprs()){
            io_strs.push_back(output_str);
        }
        for (const std::string& input_str : _genWirePools[WIRE_AUTO_GEN_INPUT].get_oprs()){
            io_strs.push_back(input_str);
        }
        for (const std::string& output_str: _genWirePools[WIRE_AUTO_GEN_OUTPUT].get_oprs()){
            io_strs.push_back(output_str);
        }
        io_strs.emplace_back("clk");
        bool is_first = true;
        for (const std::string& ret_str: io_strs){
            if (!is_first){
                result += ",\n";
            }
            result += ret_str;
            is_first = false;
        }

        result += ");\n";
        return result;
    }

    std::string ModuleGen::get_opr(){
        return _master->get_global_name() + "_" + _master->get_var_name();
    }


}