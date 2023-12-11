//
// Created by tanawin on 10/12/2566.
//

#ifndef KATHRYN_STATEREGUTIL_H
#define KATHRYN_STATEREGUTIL_H


#include "model/FlowBlock/abstract/stateReg.h"

namespace kathryn{

    std::vector<Operable*> getDependState(Operable* dependOpr){
        if (dependOpr == nullptr){
            return {};
        }
        Identifiable* ident = dependOpr->castToIdent();
        HW_COMPONENT_TYPE hwType = ident->getType();

        switch (hwType){

            case TYPE_REG:
            case TYPE_STATE_REG:
            case TYPE_WIRE:
            case TYPE_VAL:
                return {dependOpr};
            case TYPE_EXPRESSION: {
                auto *expr = dynamic_cast<expression *>(ident);
                auto preResult0= getDependState(expr->getOperandA());
                auto preResult1 = getDependState(expr->getOperandB());
                preResult0.insert(preResult0.end(), preResult1.begin(), preResult1.end());
                return preResult0;
            }
            default:
                assert(true);
        }
        return {};
    }



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

    std::vector<StResMeta> getStateInfo(StateReg* stRegPtr){

        std::vector<StResMeta> preResult;
        auto& updateMetas = stRegPtr->getUpdateMeta();

        for (auto upEvent : updateMetas){
            /////// get update event for that state register
            StResMeta result;
            result.condition = upEvent->updateCondition;
            /** recursively get state as */
            result.dependState = getDependState(upEvent->updateState);
            preResult.push_back(result);
        }

        return preResult;
    }

}

#endif //KATHRYN_STATEREGUTIL_H
