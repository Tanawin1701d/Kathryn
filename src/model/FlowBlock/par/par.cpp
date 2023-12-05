//
// Created by tanawin on 4/12/2566.
//

#include "par.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockPar::FlowBlockPar(): FlowBlockBase(PARALLEL) {


    }

    void
    FlowBlockPar::addElementInFlowBlock(Node* node){
        assert(node != nullptr);
        _simpleAsm.push_back(node);
    }


    NodeWrapper*
    FlowBlockPar::sumarizeBlock() {
        assert(outputExpression != nullptr);
        assert(!entranceEvent.empty());
        return new NodeWrapper({entranceEvent, outputExpression});
    }


    void
    FlowBlockPar::onAttachBlock(){
        ctrl->on_attach_flowBlock(this);
    }

    void
    FlowBlockPar::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void
    FlowBlockPar::buildHwComponent() {
        assert((!_simpleAsm.empty()) || (!subBlocks.empty()));

        /** build sub component*/
        for (auto sb : subBlocks){
            sb->buildHwComponent();
        }
        /** initialize output expression*/
        outputExpression = new expression();

        /** state for basic assignment */
        if (!_simpleAsm.empty()) {
            stReg = new StateReg();
            entranceEvent.push_back(stReg->genUpdateEvent());
        }
        /** state for subblock and implement output expression*/
        if (!subBlocks.empty()){
            /** for now we know that synchronization is need*/
            if (stReg != nullptr) {
                /** build synchonizer for basic block*/
                auto synStateForBasic = new StateReg();
                synStateForBasic->genUpdateEvent(stReg);
                synState.push_back(synStateForBasic);
            }
            /** for subblock synchonization*/
            for (auto sb: subBlocks){
                auto wrapper = sb->sumarizeBlock();
                for (auto subEntrance: wrapper->entranceElements){
                    assert(subEntrance != nullptr);
                    entranceEvent.push_back(subEntrance);
                }
                auto synSt = new StateReg();
                synSt->genUpdateEvent(wrapper->exitExpr);
                synState.push_back(synSt);
                delete wrapper;

            }
            /** wrap synchronization to output expression*/
            assert(!synState.empty());
            *outputExpression = *synState[0];
            for (int i = 1; i < synState.size(); i++){
                outputExpression = &((*outputExpression) | (*synState[i]));
            }

        }else{
            assert(stReg != nullptr);
            *outputExpression = *stReg;
        }
    }

    void
    FlowBlockPar::doPreFunction() {
        onAttachBlock();
    }

    void
    FlowBlockPar::doPostFunction(){
        onDetachBlock();
    }



}
