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


#include "model/flowBlock/abstract/spReg/stateReg.h"

namespace kathryn{

    struct StResMeta{
        Operable* condition = nullptr;
        std::vector<Operable*> dependState;

        std::string getDebugString(){
            std::string result =  "depend on state ";
            for (auto dep: dependState){
                result += dep->castToIdent()->get_global_name() + ",";
            }
            result += "when " + ((condition != nullptr) ?
                    condition->castToIdent()->get_global_name() :
                    "none"
                    );
            return result;

        }
    };

    std::vector<StResMeta> getStateInfo(StateReg* stRegPtr);

    std::vector<Operable*> getDependState(Operable* dependOpr);

}

#endif //KATHRYN_STATEREGUTIL_H
