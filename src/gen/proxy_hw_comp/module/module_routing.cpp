//
// Created by tanawin on 1/7/2024.
//

#include "model/hw_component/wire/wire_auto.h"
#include "model/hw_component/module/module.h"

namespace kathryn{

    void ModuleGen::start_route_ele(){
        /////////// route io
        if (_master->is_top_module()){
            ////////////////////////// recruit first
            LogicGenBaseVec input_logic_gen_base;
            LogicGenBaseVec output_logic_gen_base;
            recruit_logic_gen_base(input_logic_gen_base , _genWires[WIRE_AUTO_GEN_GLOB_INPUT]);
            recruit_logic_gen_base(output_logic_gen_base, _genWires[WIRE_AUTO_GEN_GLOB_OUTPUT]);
            /////////////////////////// route dep for global io
            input_logic_gen_base.route_dep_all();
            output_logic_gen_base.route_dep_all();
        }
        /////////// module gen
        for(ModuleGen* md_gen: _subModulePool){
            md_gen->start_route_ele();
        }
        /////////// module gen
        _regPool                 .route_dep_all();
        _wirePool                .route_dep_all();
        _wirePoolWithInputMarker .route_dep_all();
        _wirePoolWithOutputMarker.route_dep_all();
        _exprPool                .route_dep_all();
        _nestPool                .route_dep_all();
        _valPool                 .route_dep_all(); //// pm doesn't require
        _memBlockPool            .route_dep_all();
        _memBlockElePool         .route_dep_all();
    }

    void ModuleGen::finalize_route_ele() {
        for(ModuleGen* md_gen: _subModulePool){
            md_gen->finalize_route_ele();
        }
        for(int gen_wire_type = 0; gen_wire_type < WIRE_AUTO_GEN_CNT; gen_wire_type++){
            recruit_logic_gen_base(
                _genWirePools[gen_wire_type], _genWires[gen_wire_type]);
        }
    }

    WireAuto* ModuleGen::gen_auto_wire_base(
        Operable* opr1,      ///////// opr is exact opr
        Operable* real_src,  ///////// real_src is exact opr too
        const std::string& wire_name,
        WIRE_AUTO_GEN_TYPE wire_gen_type,
        bool connect_the_wire){

        assert(opr1     != nullptr);
        assert(real_src != nullptr);
        assert(wire_gen_type < WIRE_AUTO_GEN_CNT);

        std::unordered_map<Operable*, int>& gen_map = _genWireMaps[wire_gen_type];
        std::vector<WireAuto*>&          gen_vec = _genWires[wire_gen_type];

        WireAuto& new_added_wire = make_opr_io_wire("addAutoWireBase_uninit",
                                              opr1->get_operable_slice().get_size(),
                                              wire_gen_type);

            ///////// addd update Event for only connection
            new_added_wire.build_hierarchy(_master);
            new_added_wire.create_logic_gen();
            if (connect_the_wire){
                new_added_wire.connect_to(opr1, true);
            }

            new_added_wire.set_var_name(wire_name +
                                    std::to_string(gen_vec.size()) +
                                    "_" +
                                    real_src->get_logic_gen_ptrBase()->
                                    get_ident_ptr()->get_var_name()
                                    );
            gen_vec.push_back(&new_added_wire);
            gen_map.insert({real_src, gen_vec.size()-1});
            //////////////////////////////////////////////
            return &new_added_wire;
    }

    bool ModuleGen::is_there_auto_gen_wire(Operable* real_src, WIRE_AUTO_GEN_TYPE wire_gen_type){
        std::unordered_map<Operable*, int>& gen_map = _genWireMaps[wire_gen_type];
        return gen_map.find(real_src) != gen_map.end();
    }

    WireAuto* ModuleGen::get_auto_gen_wire(Operable* real_src, WIRE_AUTO_GEN_TYPE wire_gen_type){
        std::unordered_map<Operable*, int>& gen_map = _genWireMaps[wire_gen_type];
        std::vector<WireAuto*>&          gen_vec = _genWires[wire_gen_type];
        return gen_vec.at(gen_map[real_src]);
    }

    /////////////////////////////// routing wire

    Operable* ModuleGen::route_src_opr_to_this_module(Operable* real_src){
        assert(real_src != nullptr);

        Operable* exact_real_src = &real_src->get_exact_operable();

        ModuleGen* des_module_gen = this;
        ModuleGen* src_module_gen = exact_real_src->get_logic_gen_ptrBase()->get_module_gen_ptr();

        ////// if it is same module then return that operable
        if (src_module_gen == des_module_gen){
            return real_src;
        }
        //////// check is it output of the submodule and it is user output wire
        Module* parent_src_module = src_module_gen->get_master_module_ptr()->get_parent_ptr();
        if ((parent_src_module != nullptr) &&
            (parent_src_module->get_module_gen_ptr() == des_module_gen)){ //// check that src is submodule of this module
                Module* sub_module = src_module_gen->get_master_module_ptr();
                for (Wire* output_wire: sub_module->get_user_wires_by_marker(WMT_OUTPUT_MD)){
                    if (exact_real_src == ((Operable*)output_wire)){
                        ///////// if it is match return real_src
                        return real_src;
                    }
                }
        }

        //// now it must be routed in some way

        std::vector<ModuleGen*> use_input_as_module_gen;
        std::vector<ModuleGen*> use_output_as_module_gen;
        //// do routing until it match
        while (des_module_gen != src_module_gen){
            int des_depth = des_module_gen->get_dept();
            int src_depth = src_module_gen->get_dept();
            //////// Iterate to upper module
            if (des_depth > src_depth){
                use_input_as_module_gen.push_back(des_module_gen);
                des_module_gen = des_module_gen->_master->get_parent_ptr()->get_module_gen_ptr();
            }else if (des_depth < src_depth){
                use_output_as_module_gen.push_back(src_module_gen);
                src_module_gen = src_module_gen->_master->get_parent_ptr()->get_module_gen_ptr();
            }else{
                use_input_as_module_gen.push_back(des_module_gen);
                use_output_as_module_gen.push_back(src_module_gen);
                des_module_gen = des_module_gen->_master->get_parent_ptr()->get_module_gen_ptr();
                src_module_gen = src_module_gen->_master->get_parent_ptr()->get_module_gen_ptr();
            }
        }
        use_input_as_module_gen.push_back(des_module_gen);
        use_output_as_module_gen.push_back(src_module_gen);


        /////////////////////////////////////
        /// inter wire
        /////////////////////////////////////
        ModuleGen* apogee    = *use_input_as_module_gen.rbegin();
        WireAuto*    inter_wire = nullptr;
        if (apogee->is_there_auto_gen_wire(exact_real_src, WIRE_AUTO_GEN_INTER)){
            inter_wire = apogee->get_auto_gen_wire(exact_real_src, WIRE_AUTO_GEN_INTER);
        }else{
            inter_wire = apogee->gen_auto_wire_base(
                exact_real_src, exact_real_src,
                "ABD_", WIRE_AUTO_GEN_INTER,
                false);
        }
        ////////////////////////////////////
        ///
        /// des series do it as input
        /// we assure the vector have at least one element in size
        /// //////////////////////////////////
        WireAuto* input_wire = inter_wire;
        for(int idx = ((int)use_input_as_module_gen.size()-2); idx >= 0; idx--){
            ModuleGen& cur_md_gen = *use_input_as_module_gen[idx];
            if (cur_md_gen.is_there_auto_gen_wire(exact_real_src, WIRE_AUTO_GEN_INPUT)){
                input_wire = cur_md_gen.get_auto_gen_wire(exact_real_src, WIRE_AUTO_GEN_INPUT);
                continue;
            }
            input_wire = cur_md_gen.gen_auto_wire_base(
                input_wire, exact_real_src,
                "AIP_", WIRE_AUTO_GEN_INPUT,
                true);
        }


        ///////////////////////////////////
        ///
        ///src series do it as output
        ///
        ///
        ////////////////////////////////////
        Operable* output_wire = exact_real_src;
        for(int idx = 0; idx < ((int)use_output_as_module_gen.size()-1); idx++){
            ModuleGen& cur_md_gen = *use_output_as_module_gen[idx];
            if (cur_md_gen.is_there_auto_gen_wire(exact_real_src, WIRE_AUTO_GEN_OUTPUT)){
                output_wire = cur_md_gen.get_auto_gen_wire(exact_real_src, WIRE_AUTO_GEN_OUTPUT);
                continue;
            }
            output_wire = cur_md_gen.gen_auto_wire_base(
                output_wire, exact_real_src,
                "AOP_", WIRE_AUTO_GEN_OUTPUT,
                true);
        }
        inter_wire->connect_to(output_wire, true);
        //////////////////////////////

        if (real_src->get_operable_slice() == exact_real_src->get_operable_slice()){
            return input_wire;
        }
        ////// it is exact operable
        return &((*input_wire)
                (real_src->get_operable_slice()));
    }

}