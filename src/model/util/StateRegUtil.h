//
// Created by tanawin on 10/12/2566.
//

#ifndef KATHRYN_STATEREGUTIL_H
#define KATHRYN_STATEREGUTIL_H


#include "model/FlowBlock/abstract/spReg/stateReg.h"

namespace kathryn{

    struct StResMeta{
        Operable* condition = nullptr;
        std::vector<Operable*> dependState;

        std::string getDebugString(){
            std::string result =  "depend on state ";
            for (auto dep: dependState){
                result += dep->castToIdent()->getGlobalName() + ",";
            }
            result += "when " + ((condition != nullptr) ?
                    condition->castToIdent()->getGlobalName() :
                    "none"
                    );
            return result;

        }
    };

    std::vector<StResMeta> getStateInfo(StateReg* stRegPtr);

    std::vector<Operable*> getDependState(Operable* dependOpr);

}

#endif //KATHRYN_STATEREGUTIL_H
