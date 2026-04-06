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
        _entNode      ->set_internal_ident("entNode_"+std::to_string(masterIdx)+"_"+std::to_string(subIdx));
        _waitPrevNode ->set_internal_ident("waitPrevNode_"+std::to_string(masterIdx)+"_"+std::to_string(subIdx));
        _waitNextNode ->set_internal_ident("waitNextNode_"+std::to_string(masterIdx)+"_"+std::to_string(subIdx));
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

        _acceptForPrev->add_depend_node(_syncedNext  , nullptr);
        _acceptForPrev->add_depend_node(_waitPrevNode, nullptr);
        _acceptForPrev->assign();

        _readyForNext->add_depend_node(_waitNextNode                       , nullptr);
        _readyForNext->add_depend_node(_masterElement->getStateFinishIden(), nullptr);
        _readyForNext->assign();


        _syncedNext->add_depend_node(_readyForNext, nullptr);
        if (nextStreamEle != nullptr){
            _syncedNext->add_depend_node(nextStreamEle->_acceptForPrev, nullptr);
        }
        _syncedNext->assign();

        //////// create next wait node
        if (nextStreamEle != nullptr){
            Operable* notAcceptNext = &(~(*nextStreamEle->_acceptForPrev->get_exit_opr_ptr()));
            _waitNextNode->add_depend_node(_masterElement->getStateFinishIden(),
                                         notAcceptNext);
            _waitNextNode->add_depend_node(_waitNextNode, notAcceptNext);
        }
        tryAssignIntSig(_waitNextNode);
        tryAssignHoldSig(_waitNextNode);
        _waitNextNode->assign();

        //////// create prev wait node
        Operable* prevNotReady = nullptr;
        Operable* prevReady    = nullptr;
        if (prevStreamEle != nullptr){
            prevReady    = prevStreamEle->_readyForNext->get_exit_opr_ptr();
            prevNotReady = &(~(*prevReady));

            _waitPrevNode->add_depend_node(_syncedNext, prevNotReady);
            _waitPrevNode->add_depend_node(_waitPrevNode, prevNotReady);
            tryAssignIntSig(_waitNextNode);
            tryAssignHoldSig(_waitNextNode);
            ///_waitPrevNode->assign(); //// because we have to send to node wrapper
        }
        //////// create entNode
        _entNode->add_depend_node(_waitPrevNode, prevReady);
        _entNode->add_depend_node(_syncedNext, prevReady);
        _entNode->assign();

        //////// add main flow to entNode
        _masterElement->assignDependDent(_entNode);
    }

    void StreamEle::assignIntStart(OprNode* intStartNode){
        _waitPrevNode->add_depend_node(intStartNode, nullptr);
    }

    void StreamEle::addToSystemNodes(std::vector<Node*>& sysNode){
        assert(_waitPrevNode != nullptr && _waitNextNode != nullptr);
        sysNode.push_back(_waitPrevNode);
        sysNode.push_back(_waitNextNode);
    }

    void StreamEle::tryAssignIntSig(StateNode* nd) const{
        if (_masterElement->getIntResetNode() != nullptr){
            nd->set_interrupt_reset(_masterElement->getIntResetNode());
        }
    }

    void StreamEle::tryAssignHoldSig(StateNode* nd) const{
        if (_masterElement->getHoldNode() != nullptr ){
            nd->set_hold(_masterElement->getHoldNode());
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


    Node* StreamEle::getEntranceNode()const{
        return _waitPrevNode;
    }


}