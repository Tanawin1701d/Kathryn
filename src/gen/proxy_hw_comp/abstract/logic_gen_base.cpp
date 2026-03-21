//
// Created by tanawin on 20/6/2024.
//

#include "logic_gen_base.h"

#include "utility"
#include "gen/proxy_hw_comp/module/module_gen.h"

namespace kathryn{


    LogicGenBase::LogicGenBase(ModuleGen*    md_gen_master,
                               Assignable*   asb,
                               Identifiable* ident):
_mdGenMaster(md_gen_master),
_asb(asb),
_ident(ident){
    assert(md_gen_master != nullptr);
}

std::string LogicGenBase::get_opr_str_from_opr(Operable* opr1){
    assert(opr1 != nullptr);
    return opr1->get_exact_operable().
        get_logic_gen_ptrBase()->get_opr(opr1->get_operable_slice());
}

std::string LogicGenBase::get_opr_str_from_opr_and_shink_msb(Operable* opr1, int target_size){

    int src_size = opr1->get_operable_slice().get_size();
    assert(target_size <= src_size);
    assert(target_size > 0);
    std::string pure_src_str = get_opr_str_from_opr(opr1);

    if (target_size == src_size){
        return pure_src_str;
    }
    return pure_src_str + "[" + std::to_string(target_size-1) + ": 0]";

}

std::string LogicGenBase::get_opr(){
    assert(_ident != nullptr);
    // if (!_ident->is_user_var()){
    //     return _ident->get_global_name();
    // }
    return _ident->get_global_name() + "_" + _ident->get_var_name();

}

std::string LogicGenBase::get_opr(Slice sl){
        assert(sl.check_valid_slice());
        if (sl == _asb->get_assign_slice()){
            return get_opr();
        }
        return get_opr() + "[" + std::to_string(sl.stop-1) +
               ": " + std::to_string(sl.start) + "]";
}
}
