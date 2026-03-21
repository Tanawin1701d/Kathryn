//
// Created by tanawin on 10/12/2566.
//

#ifndef KATHRYN_STATEREGUTIL_H
#define KATHRYN_STATEREGUTIL_H


/***
 *
 * lagacy
 *
 * */


#include "model/flow_block/abstract/sp_reg/state_reg.h"

namespace kathryn{

    struct StResMeta{
        Operable* condition = nullptr;
        std::vector<Operable*> depend_state;

        std::string get_debug_string(){
            std::string result =  "depend on state ";
            for (auto dep: depend_state){
                result += dep->cast_to_ident()->get_global_name() + ",";
            }
            result += "when " + ((condition != nullptr) ?
                    condition->cast_to_ident()->get_global_name() :
                    "none"
                    );
            return result;

        }
    };

    std::vector<StResMeta> get_state_info(StateReg* st_reg_ptr);

    std::vector<Operable*> get_depend_state(Operable* depend_opr);

}

#endif //KATHRYN_STATEREGUTIL_H
