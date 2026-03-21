//
// Created by tanawin on 11/10/25.
//

#ifndef SRC_SIM_MODELSIMENGINE_FLOWBLOCK_PIPELINE_FLOWBLOCKPIPPROBER_H
#define SRC_SIM_MODELSIMENGINE_FLOWBLOCK_PIPELINE_FLOWBLOCKPIPPROBER_H
#include "sim/model_sim_engine/flow_block/flow_block_prober.h"

namespace kathryn{

    class FlowBlockPipeBase;
    struct PipSimProbe: SimProbe{
        FlowBlockPipeBase* flow_block_pip_base = nullptr;

        void init(FlowBlockPipeBase* flow_block_pip);

        bool is_idle()      override;
        bool is_waiting()   override;
        bool is_executing() override;

    };


    class FlowBlockZyncBase;
    struct ZyncSimProb: SimProbe{

        FlowBlockZyncBase* flow_block_zync_base = nullptr;

        bool get_prep_send_node_status();
        bool get_zync_ready_status();

        void init(FlowBlockZyncBase* flow_block_zync);
        bool is_idle     () override;
        bool is_waiting  () override;
        bool is_executing() override;


    };

}

#endif //SRC_SIM_MODELSIMENGINE_FLOWBLOCK_PIPELINE_FLOWBLOCKPIPPROBER_H
