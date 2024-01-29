//
// Created by tanawin on 11/12/2566.
//

#include"StateRegUtil.h"

namespace kathryn{


    std::vector<StResMeta> getStateInfo(StateReg* stRegPtr){

        std::vector<StResMeta> preResult;
        auto& updateMetas = stRegPtr->getUpdateMeta();

        for (auto upEvent : updateMetas){
            /////// get update event for that state register
            StResMeta result;
            result.condition = upEvent->srcUpdateCondition;
            /** recursively get state as */
            result.dependState = getDependState(upEvent->srcUpdateState);
            preResult.push_back(result);
        }

        return preResult;
    }



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

}