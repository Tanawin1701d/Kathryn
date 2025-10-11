//
// Created by tanawin on 11/10/25.
//

#ifndef SRC_SIM_MODELSIMENGINE_FLOWBLOCK_FLOWBLOCKPROBER_H
#define SRC_SIM_MODELSIMENGINE_FLOWBLOCK_FLOWBLOCKPROBER_H


#define initProbe(x) x.init(kathrynBlock)

namespace kathryn{

    /////// this class aims to help the designer to profile the data from the flow block without
    ////// interfere with the flow block simulation engine




    class FlowBlockBase;
    struct SimProbe{
        virtual ~SimProbe() = default;

        FlowBlockBase* _flowBlockBase = nullptr;

        void init(FlowBlockBase* flowBlockBase);

        bool checkSimEngineReady();

        virtual bool isIdle();      ////// there is no action right now
        virtual bool isWaiting();   ////// system is waiting to execute something
        virtual bool isExecuting(); ////// system is executing

    };


}

#endif //SRC_SIM_MODELSIMENGINE_FLOWBLOCK_FLOWBLOCKPROBER_H
