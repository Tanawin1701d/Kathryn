//
// Created by tanawin on 9/12/25.
//

#ifndef MODEL_HWCOLLECTION_DATASTRUCTURE_MUX_MUX_H
#define MODEL_HWCOLLECTION_DATASTRUCTURE_MUX_MUX_H
#include "model/flowBlock/abstract/nodes/asmNode.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/wire/wire.h"

namespace kathryn{


    inline Operable& mux(Operable& sel, Operable& x0, Operable& x1){

        assert(sel.getOperableSlice().getSize() == 1);
        assert(x0.getOperableSlice().getSize() == x1.getOperableSlice().getSize());
        int destinatedWidth = x0.getOperableSlice().getSize();
        assert(destinatedWidth > 0);

        Wire& result = mOprWire("mux2", destinatedWidth);

        AssignMeta* sel0 =
        result.generateAssignMeta(x0, result.getOperableSlice(), ASM_DIRECT, CM_CLK_FREE);
        AssignMeta* sel1 =
        result.generateAssignMeta(x1, result.getOperableSlice(), ASM_DIRECT, CM_CLK_FREE);

        AsmNode* assNode = new AsmNode({sel0, sel1});
        assNode->addSpecificPreCondition(&(sel == 0), 0);
        assNode->addSpecificPreCondition(&(sel == 1), 1);
        assNode->dryAssign();
        delete assNode;

        return result;


    }

}

#endif //MODEL_HWCOLLECTION_DATASTRUCTURE_MUX_MUX_H