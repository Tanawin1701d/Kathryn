//
// Created by tanawin on 7/2/26.
//

#include "streamEle.h"

namespace kathryn{

    StreamEle::StreamEle(SequenceEle* masterEle):
    _masterElement(masterEle){}

    StreamEle::~StreamEle(){
        delete _masterElement;
        delete _entNode;
        delete _waitPrevNode;
        delete _waitNextNode;
        delete _acceptForPrev;
        delete _readyForNext;
        delete _syncedNext;
    }

    void StreamEle::genNode(CLOCK_MODE cm){
        ///// genNode in seq element
        assert(_masterElement != nullptr);
        ///// gen the sequential element

        ///// gen the pipeline ctrl
        _waitPrevNode  = new StateNode(cm);
        _entNode       = new PseudoNode(1, BITWISE_OR);
        _acceptForPrev = new PseudoNode(1, BITWISE_OR);

        _masterElement->genNode(cm);

        _waitNextNode  = new StateNode(cm);
        _readyForNext  = new PseudoNode(1, BITWISE_OR);
        _syncedNext    = new PseudoNode(1, BITWISE_AND);
    }

    void StreamEle::setIdentStateId(ull masterIdx, int subIdx) const{
        _masterElement->setIdentStateId(masterIdx, subIdx);
        _entNode      ->setInternalIdent("entNode_"+std::toString(masterIdx)+"_"+std::toString(subIdx));
        _waitPrevNode ->setInternalIdent("waitPrevNode_"+std::toString(masterIdx)+"_"+std::toString(subIdx));
        _waitNextNode ->setInternalIdent("waitNextNode_"+std::toString(masterIdx)+"_"+std::toString(subIdx));
    }
    void StreamEle::setIntReset(OprNode* intResetNode){
        _masterElement->setIntReset(intResetNode);
    }
    void StreamEle::setHoldNode(OprNode* holdNode){
        _masterElement->setHoldNode(holdNode);
    }
    void StreamEle::addSyncDependency(StreamEle* prevStreamEle,
                                      StreamEle* nextStreamEle) const{

        /////// create communicate channel

        _acceptForPrev->addDependNode(_syncedNext  , nullptr);
        _acceptForPrev->addDependNode(_waitPrevNode, nullptr);
        _acceptForPrev->assign();

        _readyForNext->addDependNode(_waitNextNode                       , nullptr);
        _readyForNext->addDependNode(_masterElement->getStateFinishIden(), nullptr);
        _readyForNext->assign();


        _syncedNext->addDependNode(_readyForNext, nullptr);
        if (nextStreamEle != nullptr){
            _syncedNext->addDependNode(nextStreamEle->_acceptForPrev, nullptr);
        }
        _syncedNext->assign();

        //////// create next wait node
        if (nextStreamEle != nullptr){
            Operable* notAcceptNext = &(~(*nextStreamEle->_acceptForPrev->getExitOprPtr()));
            _waitNextNode->addDependNode(_masterElement->getStateFinishIden(),
                                         notAcceptNext);
            _waitNextNode->addDependNode(_waitNextNode, notAcceptNext);
        }
        tryAssignIntSig(_waitNextNode);
        tryAssignHoldSig(_waitNextNode);
        _waitNextNode->assign();

        //////// create prev wait node
        Operable* prevNotReady = nullptr;
        Operable* prevReady    = nullptr;
        if (prevStreamEle != nullptr){
            prevReady    = prevStreamEle->_readyForNext->getExitOprPtr();
            prevNotReady = &(~(*prevReady));

            _waitPrevNode->addDependNode(_syncedNext, prevNotReady);
            _waitPrevNode->addDependNode(_waitPrevNode, prevNotReady);
            tryAssignIntSig(_waitNextNode);
            tryAssignHoldSig(_waitNextNode);
            ///_waitPrevNode->assign(); //// because we have to send to node wrapper
        }
        //////// create entNode
        _entNode->addDependNode(_waitPrevNode, prevReady);
        _entNode->addDependNode(_syncedNext, prevReady);
        _entNode->assign();

        //////// add main flow to entNode
        _masterElement->assignDependDent(_entNode);
    }

    void StreamEle::assignIntStart(OprNode* intStartNode){
        _waitPrevNode->addDependNode(intStartNode, nullptr);
    }

    void StreamEle::addToSystemNodes(std::vector<Node*>& sysNode){
        assert(_waitPrevNode != nullptr && _waitNextNode != nullptr);
        sysNode.pushBack(_waitPrevNode);
        sysNode.pushBack(_waitNextNode);
    }

    void StreamEle::tryAssignIntSig(StateNode* nd) const{
        if (_masterElement->getIntResetNode() != nullptr){
            nd->setInterruptReset(_masterElement->getIntResetNode());
        }
    }

    void StreamEle::tryAssignHoldSig(StateNode* nd) const{
        if (_masterElement->getHoldNodePtr() != nullptr ){
            nd->setHold(_masterElement->getHoldNodePtr());
        }
    }

    bool StreamEle::checkAllNodeGen(){
        return (_masterElement != nullptr) &&
               (_entNode       != nullptr) &&
               (_waitPrevNode  != nullptr) &&
               (_waitNextNode  != nullptr) &&
               (_acceptForPrev != nullptr) &&
               (_readyForNext  != nullptr) &&
               (_syncedNext    != nullptr);
    }


    Node* StreamEle::getEntranceNodePtr()const{
        return _waitPrevNode;
    }


}