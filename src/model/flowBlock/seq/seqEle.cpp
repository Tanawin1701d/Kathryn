//
// Created by tanawin on 7/2/26.
//

#include "seqEle.h"

#include "model/flowBlock/abstract/flowBlock_Base.h"


namespace kathryn{
    

    void SequenceEle::setIntReset(OprNode* intResetNode){
        _intRstNode = intResetNode;
    }

    void SequenceEle::setHoldNode(OprNode* holdNode){
        _holdNode = holdNode;
    }

    /**
     * SequenceEleBasic
     */
    SequenceEleBasic::SequenceEleBasic(Node *asmNode) : SequenceEle(){
        assert(asmNode != nullptr);
        assert(asmNode->get_node_type() == ASM_NODE);
        _asmNode = (AsmNode*)asmNode;
    }
    SequenceEleBasic::~SequenceEleBasic(){delete _stateNode;}

    void SequenceEleBasic::genNode(CLOCK_MODE cm){
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

    void SequenceEleBasic::setIdentStateId (ull masterIdx, int idx) const{
        assert(_stateNode != nullptr);
        _stateNode->set_internal_ident(
                    "seqStateReg_"+
                    std::to_string(masterIdx)+
                    "_" +
                    std::to_string(idx));
    }

    void SequenceEleBasic::addToCycleDet(NodeWrapCycleDet& deter) const{
        deter.add_to_det(_asmNode);
    }
    void SequenceEleBasic::assignDependDent(SequenceEle* predecessor) const{
        _stateNode->add_depend_node(predecessor->getStateFinishIden(), nullptr);
        _stateNode->assign();   ///// assign state node to actual value

    }
    void SequenceEleBasic::assignIntStart(OprNode* intStartNode){
        _stateNode->add_depend_node(intStartNode, nullptr);
    }
    Node* SequenceEleBasic::getStateFinishIden() const{
        assert(_stateNode != nullptr);
        return _stateNode;
    }
    std::vector<Node*> SequenceEleBasic::getEntranceNodes(){
        assert(_stateNode != nullptr);
        return {_stateNode};
    }
    bool SequenceEleBasic::isThereForceExitNode() const{
        return false;
    }
    Node* SequenceEleBasic::getForceExitNode() const{
        assert(false);
        return nullptr;
    }
    bool SequenceEleBasic::isNodeWrap() const{
        return false;
    }
    NodeWrap* SequenceEleBasic::getNodeWrap() const{
        assert(false);
        return nullptr;
    }
    bool SequenceEleBasic::isBasicNode() const{
        return true;
    }
    StateNode* SequenceEleBasic::getBasicNode() const{
        return _stateNode;
    }
    std::string SequenceEleBasic::getDescribe(){
        return _stateNode->get_md_ident_val() + " " + _stateNode->get_md_describe();
    }
    void SequenceEleBasic::addToSystemNodes(std::vector<Node*>& sysNode){
        assert(_stateNode != nullptr);
        sysNode.push_back(_stateNode);
    }

    void SequenceEleBasic::assignDependDent    (Node* activatorNode) const{
        _stateNode->add_depend_node(activatorNode, nullptr);
        _stateNode->assign();   ///// assign state node to actual value
    }


    /**
     * SequenceEleFlowBlock
     */

    SequenceEleFlowBlock::SequenceEleFlowBlock(FlowBlockBase* fbBase): SequenceEle(){
        assert(fbBase != nullptr);
        _subBlock  = fbBase;
    }

    void SequenceEleFlowBlock::genNode(CLOCK_MODE cm){
        assert(_subBlock != nullptr);
        _complexNode = _subBlock->sumarize_block();
    }

    void SequenceEleFlowBlock::setIdentStateId (ull masterIdx, int idx) const{
        assert(_complexNode != nullptr);
    }

    void SequenceEleFlowBlock::addToCycleDet(NodeWrapCycleDet& deter) const{
        deter.add_to_det(_complexNode);
    }
    void SequenceEleFlowBlock::assignDependDent(SequenceEle* predecessor) const{
        _complexNode->add_depend_node_to_all_node(predecessor->getStateFinishIden());
        _complexNode->assign_all_node();
    }
    void SequenceEleFlowBlock::assignIntStart(OprNode* intStartNode){
        _complexNode->add_depend_node_to_all_node(intStartNode);
    }

    Node* SequenceEleFlowBlock::getStateFinishIden() const{
        return _complexNode->get_exit_node();
    }
    std::vector<Node*> SequenceEleFlowBlock::getEntranceNodes(){
        return _complexNode->_entrance_nodes;

    }
    bool SequenceEleFlowBlock::isThereForceExitNode() const{
        return (_complexNode != nullptr)  &&
               (_complexNode->is_there_force_exit_node());
    }
    Node* SequenceEleFlowBlock::getForceExitNode() const{
        assert(isThereForceExitNode());
        return _complexNode->get_force_exit_node();

    }
    bool SequenceEleFlowBlock::isNodeWrap() const{
        return true;
    }
    NodeWrap* SequenceEleFlowBlock::getNodeWrap() const{
        return _complexNode;
    }
    bool SequenceEleFlowBlock::isBasicNode() const{
        return false;

    }
    StateNode* SequenceEleFlowBlock::getBasicNode() const{
        assert(false);
        return nullptr;
    }
    std::string SequenceEleFlowBlock::getDescribe(){
        return _complexNode->get_md_ident_val() + _complexNode->get_md_describe();
    }
    void SequenceEleFlowBlock::addToSystemNodes(std::vector<Node*>& sysNode){}

    void SequenceEleFlowBlock::assignDependDent(Node* activatorNode) const{
        _complexNode->add_depend_node_to_all_node(activatorNode);
        _complexNode->assign_all_node();
    }


}
