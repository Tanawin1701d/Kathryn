//
// Created by tanawin on 8/10/25.
//

#ifndef SRC_MODEL_FLOWBLOCK_ABSTRACT_LOGICHELPER_H
#define SRC_MODEL_FLOWBLOCK_ABSTRACT_LOGICHELPER_H
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/expression/expression.h"


namespace kathryn{

    inline kathryn::Operable* addLogicWithOutput(Operable* opr1, Operable* opr2, LOGIC_OP op){
        assert(op == BITWISE_AND || op == BITWISE_OR);

        if ( (opr1 == nullptr) && (opr2 == nullptr)){
            return nullptr;
        }

        if(opr1 == nullptr){
            return opr2;
        }else if(opr2 == nullptr){
            return opr1;
        }else if(op == BITWISE_AND) {
            return &((*opr1) & (*opr2));
        }else if(op == BITWISE_OR) {
            return &((*opr1) | (*opr2));
        }else{
            assert(false);
        }
    }

}

#endif //SRC_MODEL_FLOWBLOCK_ABSTRACT_LOGICHELPER_H
