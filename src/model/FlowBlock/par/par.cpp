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
            /** sent update asignment to be connect to this element*/
            for (auto node : _simpleAsm){
                node->updateElement->setUpdateState(stReg);
            }
        }

        /** build syn register*/
        /** we must syn of it is empty*/
        if (!subBlocks.empty())
            synReg = new StateReg((int)subBlocks.size() + stReg != nullptr);

        /** state for subblock and implement output expression*/
        if (!subBlocks.empty()){
            /** for now we know that synchronization is need*/
            if (stReg != nullptr) {
                /** build synchonizer for basic block*/
                synReg->genUpdateEvent(stReg, 0);
            }
            /** for subblock synchonization*/
            int nextSynBit = 1;
            for (auto sb: subBlocks){
                auto wrapper = sb->sumarizeBlock();
                /** deal with input of block*/
                for (auto subEntrance: wrapper->entranceElements){
                    assert(subEntrance != nullptr);
                    entranceEvent.push_back(subEntrance);
                }
                /** deal with output of block*/
                /** don't have to use node wrapper this work with this
                 * because it is single asignment
                 * */
                synReg->genUpdateEvent(wrapper->exitExpr,
                                       nextSynBit++);
                delete wrapper;
            }
        }

        /***/
        outputExpression = synReg != nullptr ? synReg->genOutputExpression() :
                                                stReg->genOutputExpression() ;


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
