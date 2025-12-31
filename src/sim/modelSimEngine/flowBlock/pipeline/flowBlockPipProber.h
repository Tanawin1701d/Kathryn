//
// Created by tanawin on 11/10/25.
//

#ifndef SRC_SIM_MODELSIMENGINE_FLOWBLOCK_PIPELINE_FLOWBLOCKPIPPROBER_H
#define SRC_SIM_MODELSIMENGINE_FLOWBLOCK_PIPELINE_FLOWBLOCKPIPPROBER_H
#include "sim/modelSimEngine/flowBlock/flowBlockProber.h"

namespace kathryn{

    class FlowBlockPipeBase;
    struct PipSimProbe: SimProbe{
        FlowBlockPipeBase* flowBlockPipBase = nullptr;

        void init(FlowBlockPipeBase* flowBlockPip);

        bool isIdle()      override;
        bool isWaiting()   override;
        bool isExecuting() override;

    };


    class FlowBlockZyncBase;
    struct ZyncSimProb: SimProbe{

        FlowBlockZyncBase* flowBlockZyncBase = nullptr;

        bool getPrepSendNodeStatus();
        bool getZyncReadyStatus();

        void init(FlowBlockZyncBase* flowBlockZync);
        bool isIdle     () override;
        bool isWaiting  () override;
        bool isExecuting() override;


    };

}

#endif //SRC_SIM_MODELSIMENGINE_FLOWBLOCK_PIPELINE_FLOWBLOCKPIPPROBER_H
