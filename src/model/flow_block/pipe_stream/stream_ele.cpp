//
// Created by tanawin on 7/2/26.
//

#include "stream_ele.h"

namespace kathryn{

    StreamEle::StreamEle(SequenceEle* master_ele):
    _masterElement(master_ele){}

    StreamEle::~StreamEle(){
        delete _masterElement;
        delete _entNode;
        delete _waitPrevNode;
        delete _waitNextNode;
        delete _acceptForPrev;
        delete _readyForNext;
        delete _syncedNext;
    }

    void StreamEle::gen_node(CLOCK_MODE cm){
        ///// gen_node in seq element
        assert(_masterElement != nullptr);
        ///// gen the sequential element

        ///// gen the pipeline ctrl
        _waitPrevNode  = new StateNode(cm);
        _entNode       = new PseudoNode(1, BITWISE_OR);
        _acceptForPrev = new PseudoNode(1, BITWISE_OR);

        _masterElement->gen_node(cm);

        _waitNextNode  = new StateNode(cm);
        _readyForNext  = new PseudoNode(1, BITWISE_OR);
        _syncedNext    = new PseudoNode(1, BITWISE_AND);
    }

    void StreamEle::set_ident_state_id(ull master_idx, int sub_idx) const{
        _masterElement->set_ident_state_id(master_idx, sub_idx);
        _entNode      ->set_internal_ident("entNode_"+std::to_string(master_idx)+"_"+std::to_string(sub_idx));
        _waitPrevNode ->set_internal_ident("waitPrevNode_"+std::to_string(master_idx)+"_"+std::to_string(sub_idx));
        _waitNextNode ->set_internal_ident("waitNextNode_"+std::to_string(master_idx)+"_"+std::to_string(sub_idx));
    }
    void StreamEle::set_int_reset(OprNode* int_reset_node){
        _masterElement->set_int_reset(int_reset_node);
    }
    void StreamEle::set_hold_node(OprNode* hold_node){
        _masterElement->set_hold_node(hold_node);
    }
    void StreamEle::add_sync_dependency(StreamEle* prev_stream_ele,
                                      StreamEle* next_stream_ele) const{

        /////// create communicate channel

        _acceptForPrev->add_depend_node(_syncedNext  , nullptr);
        _acceptForPrev->add_depend_node(_waitPrevNode, nullptr);
        _acceptForPrev->assign();

        _readyForNext->add_depend_node(_waitNextNode                       , nullptr);
        _readyForNext->add_depend_node(_masterElement->get_state_finish_iden(), nullptr);
        _readyForNext->assign();


        _syncedNext->add_depend_node(_readyForNext, nullptr);
        if (next_stream_ele != nullptr){
            _syncedNext->add_depend_node(next_stream_ele->_acceptForPrev, nullptr);
        }
        _syncedNext->assign();

        //////// create next wait node
        if (next_stream_ele != nullptr){
            Operable* not_accept_next = &(~(*next_stream_ele->_acceptForPrev->get_exit_opr_ptr()));
            _waitNextNode->add_depend_node(_masterElement->get_state_finish_iden(),
                                         not_accept_next);
            _waitNextNode->add_depend_node(_waitNextNode, not_accept_next);
        }
        try_assign_int_sig(_waitNextNode);
        try_assign_hold_sig(_waitNextNode);
        _waitNextNode->assign();

        //////// create prev wait node
        Operable* prev_not_ready = nullptr;
        Operable* prev_ready    = nullptr;
        if (prev_stream_ele != nullptr){
            prev_ready    = prev_stream_ele->_readyForNext->get_exit_opr_ptr();
            prev_not_ready = &(~(*prev_ready));

            _waitPrevNode->add_depend_node(_syncedNext, prev_not_ready);
            _waitPrevNode->add_depend_node(_waitPrevNode, prev_not_ready);
            try_assign_int_sig(_waitNextNode);
            try_assign_hold_sig(_waitNextNode);
            ///_waitPrevNode->assign(); //// because we have to send to node wrapper
        }
        //////// create ent_node
        _entNode->add_depend_node(_waitPrevNode, prev_ready);
        _entNode->add_depend_node(_syncedNext, prev_ready);
        _entNode->assign();

        //////// add main flow to ent_node
        _masterElement->assign_depend_dent(_entNode);
    }

    void StreamEle::assign_int_start(OprNode* int_start_node){
        _waitPrevNode->add_depend_node(int_start_node, nullptr);
    }

    void StreamEle::add_to_system_nodes(std::vector<Node*>& sys_node){
        assert(_waitPrevNode != nullptr && _waitNextNode != nullptr);
        sys_node.push_back(_waitPrevNode);
        sys_node.push_back(_waitNextNode);
    }

    void StreamEle::try_assign_int_sig(StateNode* nd) const{
        if (_masterElement->get_int_reset_node() != nullptr){
            nd->set_interrupt_reset(_masterElement->get_int_reset_node());
        }
    }

    void StreamEle::try_assign_hold_sig(StateNode* nd) const{
        if (_masterElement->get_hold_node_ptr() != nullptr ){
            nd->set_hold(_masterElement->get_hold_node_ptr());
        }
    }

    bool StreamEle::check_all_node_gen(){
        return (_masterElement != nullptr) &&
               (_entNode       != nullptr) &&
               (_waitPrevNode  != nullptr) &&
               (_waitNextNode  != nullptr) &&
               (_acceptForPrev != nullptr) &&
               (_readyForNext  != nullptr) &&
               (_syncedNext    != nullptr);
    }


    Node* StreamEle::get_entrance_node_ptr()const{
        return _waitPrevNode;
    }


}