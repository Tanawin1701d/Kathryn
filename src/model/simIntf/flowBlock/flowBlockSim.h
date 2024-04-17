//
// Created by tanawin on 17/4/2567.
//

#ifndef KATHRYN_FLOWBLOCKSIM_H
#define KATHRYN_FLOWBLOCKSIM_H

#include "flowBaseSim.h"

namespace kathryn {

    class FlowBlockSimEngine : public FlowBaseSimEngine {
    protected:
        FlowBlockBase *_master = nullptr;
    public:
        explicit FlowBlockSimEngine(FlowBlockBase* master);

        void beforePrepareSim(FLOW_Meta_afterMf simMeta) override;

        void simStartCurCycle() override;

        /** sim exit cur cycle*/
        void simExitCurCycle() override;


    };

}

#endif //KATHRYN_FLOWBLOCKSIM_H
