//
// Created by tanawin on 2/3/2025.
//

#include "pipePooler.h"


namespace kathryn{


    bool PipePooler::isTherePipe(const std::string& pipeName){
        return _pipeBlks.find(pipeName) != _pipeBlks.end();
    }

    bool PipePooler::isThereTran(const std::string& tranName){
        return _tranBlks.find(tranName) != _tranBlks.end();
    }

    void PipePooler::addPipeBlk(FlowBlockPipeBase* pipeBlk){
        ////// integrity check
        assert(pipeBlk != nullptr);
        std::string pipName = pipeBlk->getPipeName();
        mfAssert(!isTherePipe(pipName), "duplicated pipe name: " + pipName);
        ////// push to system
        _pipeBlks.insert({pipName, pipeBlk});
    }

    void PipePooler::addTranBlk(FlowBlockPipeTran* tranBlk){
        /////// integrity check
        assert(tranBlk != nullptr);
        std::vector<std::string> targetNames = tranBlk->getTranTargetNames();
        for (const std::string& targetName: targetNames){
            mfAssert(!isThereTran(targetName), "duplicated pipe name: " + targetName);
            ////// push to system
            _tranBlks.insert({targetName, tranBlk});
        }
    }

    Operable* PipePooler::getPipeReadySignal(const std::string& pipeName){
        ////// integrity check
        mfAssert(isTherePipe(pipeName), "system can't find pipe name: " + pipeName);
        FlowBlockPipeBase* pipeBlk = _pipeBlks.find(pipeName)->second;
        return pipeBlk->getBlkReadySignal();
    }

    Operable* PipePooler::getTranReadySignal(const std::string& targetPipeName){
        ////// integrity check
        mfAssert(isThereTran(targetPipeName), "system can't find tran name: " + targetPipeName);
        FlowBlockPipeTran* tranBlk = _tranBlks.find(targetPipeName)->second;
        return tranBlk->getReadySignal(targetPipeName);
    }

    void PipePooler::start(){
        ///////// nothing to do
    }

    void PipePooler::reset(){
        ////////// clear all map
        _pipeBlks.clear();
        _tranBlks.clear();
    }

    void PipePooler::clean(){
        reset();
    }


    PipePooler* pipePooler = nullptr;


    PipePooler* getPipePooler(){
        if (pipePooler == nullptr){
            pipePooler = new PipePooler();
        }
        return pipePooler;
    }



}