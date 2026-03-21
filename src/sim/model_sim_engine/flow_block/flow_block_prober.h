//
// Created by tanawin on 11/10/25.
//

#ifndef SRC_SIM_MODELSIMENGINE_FLOWBLOCK_FLOWBLOCKPROBER_H
#define SRC_SIM_MODELSIMENGINE_FLOWBLOCK_FLOWBLOCKPROBER_H


#define init_probe(x) (x).init(kathryn_block)
#define try_init_probe(x) if (x != nullptr) {x->init(kathryn_block);}

namespace kathryn{

    /////// this class aims to help the designer to profile the data from the flow block without
    ////// interfere with the flow block simulation engine




    class FlowBlockBase;
    struct SimProbe{
        virtual ~SimProbe() = default;

        FlowBlockBase* _flowBlockBase = nullptr;

        void init(FlowBlockBase* flow_block_base);

        bool check_sim_engine_ready();

        virtual bool is_idle();      ////// there is no action right now
        virtual bool is_waiting();   ////// system is waiting to execute something
        virtual bool is_executing(); ////// system is executing

    };


}

#endif //SRC_SIM_MODELSIMENGINE_FLOWBLOCK_FLOWBLOCKPROBER_H
