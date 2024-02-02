//
// Created by tanawin on 2/2/2567.
//

#ifndef KATHRYN_ALWAYS_H
#define KATHRYN_ALWAYS_H

#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"
#include "model/FlowBlock/abstract/spReg/stateReg.h"
#include "model/FlowBlock/abstract/nodes/node.h"
#include "model/FlowBlock/abstract/nodes/stateNode.h"

namespace kathryn{

    class FlowBlockAlways : public FlowBlockBase, public LoopStMacro{
    protected:
        /**metas element*/
        NodeWrap* resultNodeWrap = nullptr;

        /** check*/


    };

}

#endif //KATHRYN_ALWAYS_H
