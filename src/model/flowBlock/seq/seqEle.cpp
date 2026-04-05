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
        assert(asmNode->getNodeType() == ASM_NODE);
        _asmNode = (AsmNode*)asmNode;
    }
    SequenceEleBasic::~SequenceEleBasic(){delete _stateNode;}

    void SequenceEleBasic::genNode(CLOCK_MODE cm){
        assert(_asmNode != nullptr);
        _stateNode = new StateNode(cm);
        _stateNode->addSlaveAsmNode(_asmNode);
        if (_intRstNode != nullptr) {
            _stateNode->setInterruptReset(_intRstNode);
        }
        if (_holdNode != nullptr) {
            _stateNode->setHold(_holdNode);
        }
    }

    void SequenceEleBasic::setIdentStateId (ull masterIdx, int idx) const{
        assert(_stateNode != nullptr);
        _stateNode->setInternalIdent(
                    "seqStateReg_"+
                    std::to_string(masterIdx)+
                    "_" +
                    std::to_string(idx));
    }

    void SequenceEleBasic::addToCycleDet(NodeWrapCycleDet& deter) const{
        deter.addToDet(_asmNode);
    }
    void SequenceEleBasic::assignDependDent(SequenceEle* predecessor) const{
        _stateNode->addDependNode(predecessor->getStateFinishIden(), nullptr);
        _stateNode->assign();   ///// assign state node to actual value

    }
    void SequenceEleBasic::assignIntStart(OprNode* intStartNode){
        _stateNode->addDependNode(intStartNode, nullptr);
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
        _stateNode->addDependNode(activatorNode, nullptr);
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
        _complexNode = _subBlock->sumarizeBlock();
    }

    void SequenceEleFlowBlock::setIdentStateId (ull masterIdx, int idx) const{
        assert(_complexNode != nullptr);
    }

    void SequenceEleFlowBlock::addToCycleDet(NodeWrapCycleDet& deter) const{
        deter.addToDet(_complexNode);
    }
    void SequenceEleFlowBlock::assignDependDent(SequenceEle* predecessor) const{
        _complexNode->addDependNodeToAllNode(predecessor->getStateFinishIden());
        _complexNode->assignAllNode();
    }
    void SequenceEleFlowBlock::assignIntStart(OprNode* intStartNode){
        _complexNode->addDependNodeToAllNode(intStartNode);
    }

    Node* SequenceEleFlowBlock::getStateFinishIden() const{
        return _complexNode->getExitNode();
    }
    std::vector<Node*> SequenceEleFlowBlock::getEntranceNodes(){
        return _complexNode->entranceNodes;

    }
    bool SequenceEleFlowBlock::isThereForceExitNode() const{
        return (_complexNode != nullptr)  &&
               (_complexNode->isThereForceExitNode());
    }
    Node* SequenceEleFlowBlock::getForceExitNode() const{
        assert(isThereForceExitNode());
        return _complexNode->getForceExitNode();

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
        _complexNode->addDependNodeToAllNode(activatorNode);
        _complexNode->assignAllNode();
    }


}
