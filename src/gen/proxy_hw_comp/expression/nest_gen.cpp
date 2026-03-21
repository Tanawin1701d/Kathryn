//
// Created by tanawin on 20/6/2024.
//

#include "nest_gen.h"

#include "utility"
#include "gen/proxy_hw_comp/module/module_gen.h"

#include "model/hw_component/expression/nest.h"


namespace kathryn{

    NestGen::NestGen(ModuleGen*    md_gen_master,
                     nest*         nest_master):
    LogicGenBase(md_gen_master,
                 (Assignable*) nest_master,
                 (Identifiable*) nest_master),
    _master(nest_master){
        assert(nest_master != nullptr);
    }

    void NestGen::route_dep(){
        std::vector<NestMeta> nest_list = _master->get_nest_list();
        for (NestMeta& ele: nest_list){
            assert(ele.opr1 != nullptr);
            Operable* routed_opr = _mdGenMaster->route_src_opr_to_this_module(ele.opr1);
            _routedNestList.push_back(routed_opr);
        }
    }

    std::string NestGen::dec_io(){
        assert(false);
    }

    std::string NestGen::dec_variable(){
        Slice sl = _master->get_operable_slice();

        return "wire [" + std::to_string(sl.stop-1) +
            ": 0] " + get_opr() + ";";
    }

    std::string NestGen::dec_op(){
        std::string pre_ret_str = "assign " + get_opr() + " = {";

        bool is_first = true;

        for (auto iter = _routedNestList.rbegin();
                  iter != _routedNestList.rend();
                  iter++
        ){
            if(!is_first){
                pre_ret_str += ",";
            }
            pre_ret_str += get_opr_str_from_opr(*iter);
            is_first = false;
        }

        pre_ret_str += "};";
        return pre_ret_str;
    }

}
