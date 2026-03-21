//
// Created by tanawin on 7/2/26.
//

#include "seq_ele.h"

#include "model/flow_block/abstract/flow_block__base.h"


namespace kathryn{
    

    void SequenceEle::set_int_reset(OprNode* int_reset_node){
        _intRstNode = int_reset_node;
    }

    void SequenceEle::set_hold_node(OprNode* hold_node){
        _holdNode = hold_node;
    }

    /**
     * SequenceEleBasic
     */
    SequenceEleBasic::SequenceEleBasic(Node *asm_node) : SequenceEle(){
        assert(asm_node != nullptr);
        assert(asm_node->get_node_type() == ASM_NODE);
        _asmNode = (AsmNode*)asm_node;
    }
    SequenceEleBasic::~SequenceEleBasic(){delete _stateNode;}

    void SequenceEleBasic::gen_node(CLOCK_MODE cm){
        assert(_asmNode != nullptr);
        _stateNode = new StateNode(cm);
        _stateNode->add_slave_asm_node(_asmNode);
        if (_intRstNode != nullptr) {
            _stateNode->set_interrupt_reset(_intRstNode);
        }
        if (_holdNode != nullptr) {
            _stateNode->set_hold(_holdNode);
        }
    }

    void SequenceEleBasic::set_ident_state_id (ull master_idx, int idx) const{
        assert(_stateNode != nullptr);
        _stateNode->set_internal_ident(
                    "seqStateReg_"+
                    std::to_string(master_idx)+
                    "_" +
                    std::to_string(idx));
    }

    void SequenceEleBasic::add_to_cycle_det(NodeWrapCycleDet& deter) const{
        deter.add_to_det(_asmNode);
    }
    void SequenceEleBasic::assign_depend_dent(SequenceEle* predecessor) const{
        _stateNode->add_depend_node(predecessor->get_state_finish_iden(), nullptr);
        _stateNode->assign();   ///// assign state node to actual value

    }
    void SequenceEleBasic::assign_int_start(OprNode* int_start_node){
        _stateNode->add_depend_node(int_start_node, nullptr);
    }
    Node* SequenceEleBasic::get_state_finish_iden() const{
        assert(_stateNode != nullptr);
        return _stateNode;
    }
    std::vector<Node*> SequenceEleBasic::get_entrance_nodes(){
        assert(_stateNode != nullptr);
        return {_stateNode};
    }
    bool SequenceEleBasic::is_there_force_exit_node() const{
        return false;
    }
    Node* SequenceEleBasic::get_force_exit_node_ptr() const{
        assert(false);
        return nullptr;
    }
    bool SequenceEleBasic::is_node_wrap() const{
        return false;
    }
    NodeWrap* SequenceEleBasic::get_node_wrap() const{
        assert(false);
        return nullptr;
    }
    bool SequenceEleBasic::is_basic_node() const{
        return true;
    }
    StateNode* SequenceEleBasic::get_basic_node() const{
        return _stateNode;
    }
    std::string SequenceEleBasic::get_describe(){
        return _stateNode->get_md_ident_val() + " " + _stateNode->get_md_describe();
    }
    void SequenceEleBasic::add_to_system_nodes(std::vector<Node*>& sys_node){
        assert(_stateNode != nullptr);
        sys_node.push_back(_stateNode);
    }

    void SequenceEleBasic::assign_depend_dent    (Node* activator_node) const{
        _stateNode->add_depend_node(activator_node, nullptr);
        _stateNode->assign();   ///// assign state node to actual value
    }


    /**
     * SequenceEleFlowBlock
     */

    SequenceEleFlowBlock::SequenceEleFlowBlock(FlowBlockBase* fb_base): SequenceEle(){
        assert(fb_base != nullptr);
        _subBlock  = fb_base;
    }

    void SequenceEleFlowBlock::gen_node(CLOCK_MODE cm){
        assert(_subBlock != nullptr);
        _complexNode = _subBlock->sumarize_block();
    }

    void SequenceEleFlowBlock::set_ident_state_id (ull master_idx, int idx) const{
        assert(_complexNode != nullptr);
    }

    void SequenceEleFlowBlock::add_to_cycle_det(NodeWrapCycleDet& deter) const{
        deter.add_to_det(_complexNode);
    }
    void SequenceEleFlowBlock::assign_depend_dent(SequenceEle* predecessor) const{
        _complexNode->add_depend_node_to_all_node(predecessor->get_state_finish_iden());
        _complexNode->assign_all_node();
    }
    void SequenceEleFlowBlock::assign_int_start(OprNode* int_start_node){
        _complexNode->add_depend_node_to_all_node(int_start_node);
    }

    Node* SequenceEleFlowBlock::get_state_finish_iden() const{
        return _complexNode->get_exit_node();
    }
    std::vector<Node*> SequenceEleFlowBlock::get_entrance_nodes(){
        return _complexNode->entrance_nodes;

    }
    bool SequenceEleFlowBlock::is_there_force_exit_node() const{
        return (_complexNode != nullptr)  &&
               (_complexNode->is_there_force_exit_node());
    }
    Node* SequenceEleFlowBlock::get_force_exit_node_ptr() const{
        assert(is_there_force_exit_node());
        return _complexNode->get_force_exit_node_ptr();

    }
    bool SequenceEleFlowBlock::is_node_wrap() const{
        return true;
    }
    NodeWrap* SequenceEleFlowBlock::get_node_wrap() const{
        return _complexNode;
    }
    bool SequenceEleFlowBlock::is_basic_node() const{
        return false;

    }
    StateNode* SequenceEleFlowBlock::get_basic_node() const{
        assert(false);
        return nullptr;
    }
    std::string SequenceEleFlowBlock::get_describe(){
        return _complexNode->get_md_ident_val() + _complexNode->get_md_describe();
    }
    void SequenceEleFlowBlock::add_to_system_nodes(std::vector<Node*>& sys_node){}

    void SequenceEleFlowBlock::assign_depend_dent(Node* activator_node) const{
        _complexNode->add_depend_node_to_all_node(activator_node);
        _complexNode->assign_all_node();
    }


}
