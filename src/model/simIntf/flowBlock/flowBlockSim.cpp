//
// Created by tanawin on 17/4/2567.
//

#include "flowBlockSim.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"


namespace kathryn{


    FlowBlockSimEngine::FlowBlockSimEngine(FlowBlockBase *master):
    FlowBaseSimEngine(),
    _master(master)
    {
        assert(_master != nullptr);
    }

    void FlowBlockSimEngine::beforePrepareSim(FLOW_Meta_afterMf simMeta) {
        /**set flow for this element*/
        FlowBaseSimEngine::beforePrepareSim(simMeta);
        simMeta._writer->localName = _master->getGlobalName();
        /*** invoke prepare sim in subelement in order*/
        std::vector<FlowBlockBase::sortEle> poolEle = _master->sortSubAndConFbInOrder();
        for (auto& pl: poolEle){
            ////// recurent build track element to system
            pl.fb->getFlowSimEngine()->beforePrepareSim({simMeta._writer->populateSubEle()});
        }
    }

    void FlowBlockSimEngine::simStartCurCycle(){

        /**check status*/
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        assert(_master != nullptr);
        /**check running status and inc status*/
        bool isStateRunning = false;
        /** simulate each element*/
        isStateRunning |= simStartCurCycleAndGetStatSubEle(_master->_subBlocks);
        isStateRunning |= simStartCurCycleAndGetStatSubEle(_master->_conBlocks);
        isStateRunning |= simStartCurCycleAndGetStatSubEle(_master->_sysNodes);


        /**update status*/
        /** increment log*/
        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();
        }

    }



    void FlowBlockSimEngine::simExitCurCycle(){

        unSetSimStatus();
        unsetBlockOrNodeRunning();

        simExitCurCycleSubEle(_master->_subBlocks);
        simExitCurCycleSubEle(_master->_conBlocks);
        simExitCurCycleSubEle(_master->_sysNodes);

    }





}