//
// Created by tanawin on 17/4/2567.
//

#ifndef KATHRYN_MODEL_SIMINTF_FLOWNODESIM_H
#define KATHRYN_MODEL_SIMINTF_FLOWNODESIM_H

#include "flowBaseSim.h"

namespace kathryn{

    class FlowNodeSimEngine : public FlowBaseSimEngine{
    protected:
        Node* _master = nullptr;
    public:
        explicit FlowNodeSimEngine(Node* master);
        /*** sim start simulation*/
        void simStartCurCycle() override;
        /*** sim exit simulation*/
        void simExitCurCycle() override;
    };



}

#endif //KATHRYN_MODEL_SIMINTF_FLOWNODESIM_H
