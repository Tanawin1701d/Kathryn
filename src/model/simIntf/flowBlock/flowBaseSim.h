//
// Created by tanawin on 29/3/2567.
//



#ifndef KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H
#define KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H

#include "model/simIntf/base/modelSimEngine.h"

namespace kathryn{

    /***
        *
        * flow sim interface should not sim/final  Rtl simInterface At all
        * simExitCurCycleSubEle for flow sim can be repeatly call in each cycle
        *
        * */

    class FlowBlockBase;
    struct Node;



    class FlowBaseSimEngine: public SimEngine{
    protected:
        bool _isSimulated      = false;
        int  _amtUsed          = 0;
        bool _isStateRunningIn = false; /// check that are there
                                       /// state is running in this block
        bool _isSimMetaSet     = false;

    public:
        struct FLOW_Meta_afterMf{
            FlowColEle*      _writer  = nullptr;
        };
        FLOW_Meta_afterMf _meta;

        explicit FlowBaseSimEngine();

        ~FlowBaseSimEngine() override = default;

        /**
         * sim state
         * **/
        bool isCurValSim() const override {
            return _isSimulated;
        }
        void setCurValSimStatus() override{
            _isSimulated = true;
        }
        void unSetSimStatus(){
            _isSimulated = false;
        }
        void setNextValSimStatus()       override {assert(false);}
        bool isNextValSim()        const override {assert(false);}

        /**
         * sim state
         * **/
        bool isBlockOrNodeRunning() const{
            return _isStateRunningIn;
        }
        void setBlockOrNodeRunning(){
            _isStateRunningIn = true;
        }
        void unsetBlockOrNodeRunning(){
            _isStateRunningIn = false;
        }
        void incEngine() const{
            if (_isSimMetaSet){
                assert(_meta._writer != nullptr);
                _meta._writer->freq++;
            }
        }
        /** initialize data before prepare Sim() is used*/
        virtual void beforePrepareSim(FLOW_Meta_afterMf simMeta){
            _meta         = simMeta;
            _isSimMetaSet = true;
        };
        /** start Sim can be invoked multiple times*/
        void prepareSim() override{};
        /** curCycleCollectData*/
        void curCycleCollectData() override{};
        /**simulate next cycle value for current cycle*/
        void simStartNextCycle() override{
            assert(false);
        }

        /** value overload*/
        ValRep&  getCurVal () override {assert(false);}
        ValRep&  getNextVal() override {assert(false);}

        /** sub element simulation*/
        template<typename T>
        bool simStartCurCycleAndGetStatSubEle(std::vector<T*>& blocks){
            bool isStateRunning = false;
            for (auto block: blocks){
                assert(block != nullptr);
                FlowBaseSimEngine* flowBaseSim = block->getFlowSimEngine();
                assert(flowBaseSim != nullptr);
                flowBaseSim->simStartCurCycle();
                isStateRunning |= flowBaseSim->isBlockOrNodeRunning();
            }
            return isStateRunning;
        }
        template<typename T>
        void simExitCurCycleSubEle(std::vector<T*>& blocks){

            for(auto block: blocks){
                assert(block != nullptr);
                block->getFlowSimEngine()->simExitCurCycle();
            }
        }
    };

    class FlowSimEngineInterface{
    public:
        virtual FlowBaseSimEngine* getFlowSimEngine() = 0;
    };




}

#endif //KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H
