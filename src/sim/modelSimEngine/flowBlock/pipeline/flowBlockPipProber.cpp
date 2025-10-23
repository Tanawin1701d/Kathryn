//
// Created by tanawin on 11/10/25.
//

#include "flowBlockPipProber.h"

#include <cassert>

#include "model/flowBlock/pipeline/pipe.h"
#include "model/flowBlock/pipeline/zync.h"


namespace kathryn{


    /**
     * PIPE BLOCK
     */
    void PipSimProbe::init(FlowBlockPipeBase* flowBlockPip){
        assert(flowBlockPip != nullptr);
        flowBlockPipBase = flowBlockPip;
        SimProbe::init(flowBlockPip);
    }

    bool PipSimProbe::isIdle(){
        return !isWaiting() && !isExecuting();
    }

    bool PipSimProbe::isWaiting(){
        checkSimEngineReady();
        StateNode* waitNode = flowBlockPipBase->getWaitNode();
        StateReg&  stateReg = *waitNode->_stateReg;
        bool isWaiting  = static_cast<bool>((ull)stateReg);
        return isWaiting;

    }

    bool PipSimProbe::isExecuting(){
        checkSimEngineReady();
        FlowBlockBase* implicFb = flowBlockPipBase->getImplicitFlowBlock();
        assert(implicFb != nullptr);
        return implicFb->getSimEngine()->isBlockRunning();
    }


    /**
     * ZYNC BLOCK
     **/

    bool ZyncSimProb::getPrepSendNodeStatus(){
        StateNode* preSendNode = flowBlockZyncBase->getPreSendNode();
        StateReg&  stateReg = *preSendNode->_stateReg;
        bool isPrepSend = static_cast<bool>((ull)stateReg);
        return isPrepSend;
    }

    bool ZyncSimProb::getZyncReadyStatus(){
        SyncMeta& syncMeta = flowBlockZyncBase->getSyncMeta();
        return ((ull)syncMeta._syncMatched);
    }


    void ZyncSimProb::init(FlowBlockZyncBase* flowBlockZync){
        assert(flowBlockZync != nullptr);
        flowBlockZyncBase = flowBlockZync;
        SimProbe::init(flowBlockZync);
    }

    bool ZyncSimProb::isIdle(){
        return !isWaiting() && !isExecuting();
    }

    bool ZyncSimProb::isWaiting(){
        checkSimEngineReady();
        bool isPrepSend = getPrepSendNodeStatus();
        bool isZyncReady = getZyncReadyStatus();
        return isPrepSend && (!isZyncReady);
    }

    bool ZyncSimProb::isExecuting(){
        checkSimEngineReady();
        bool isPrepSend = getPrepSendNodeStatus();
        bool isZyncReady = getZyncReadyStatus();
        return isPrepSend && isZyncReady;
    }

}
