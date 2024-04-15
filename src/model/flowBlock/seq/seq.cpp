//
// Created by tanawin on 5/12/2566.
//

#include "seq.h"
#include "model/controller/controller.h"

namespace kathryn{

    /**
     *
     *
     * sequenceElement
     *
     *
     * */


    SequenceEle::SequenceEle(Node *asmNode, OprNode* intRstNode) {
        assert(asmNode != nullptr);
        assert(asmNode->getNodeType() == ASM_NODE);
        _asmNode = (AsmNode*)asmNode;
        _intRstNode = intRstNode;
    }

    SequenceEle::SequenceEle(FlowBlockBase *fbBase) {
        assert(fbBase != nullptr);
        _subBlock  = fbBase;
    }

    SequenceEle::~SequenceEle() {
        delete _stateNode;

    }

    void SequenceEle::genNode() {

        assert( (_asmNode != nullptr) ^ (_subBlock != nullptr));

        ///// it is the basic assignment
        if (_asmNode != nullptr){
            _stateNode = new StateNode();
            _stateNode->addSlaveAsmNode(_asmNode);
            if (_intRstNode != nullptr) {
                _stateNode->setInterruptReset(_intRstNode);
            }
        }else if (_subBlock != nullptr){
            _complexNode = _subBlock->sumarizeBlock();
        }else{
            assert(false);
        }
    }

    void SequenceEle::setIdentStateId(ull masterIdx, int idx) const{
        assert((_stateNode != nullptr) ^ (_complexNode != nullptr));
        if (_stateNode != nullptr){
            _stateNode->setInternalIdent(
                    "seqStateReg_"+
                    std::to_string(masterIdx)+
                    "_" +
                    std::to_string(idx));
        }
    }

    void SequenceEle::addToCycleDet(NodeWrapCycleDet &deter) const {
        if (_asmNode != nullptr){
            deter.addToDet(_asmNode);
        } else if (_subBlock != nullptr){
            deter.addToDet(_complexNode);
        }else{
            assert(false);
        }
    }

    void SequenceEle::assignDependDent(SequenceEle *predecessor) const {
        assert(predecessor != nullptr);
        assert((_asmNode != nullptr) ^ (_subBlock != nullptr));

        if (_asmNode != nullptr){
            _stateNode->addDependNode(predecessor->getStateFinishIden(), nullptr);
            _stateNode->assign();   ///// assign state node to actual value
        }else if (_subBlock != nullptr){
            _complexNode->addDependNodeToAllNode(predecessor->getStateFinishIden());
            _complexNode->assignAllNode();
        }else{
            assert(false);
        }

    }

    void SequenceEle::assignIntStart(OprNode* intStartNode){
        assert(intStartNode != nullptr);

        if (_asmNode !=nullptr){
            _stateNode->addDependNode(intStartNode, nullptr);
        }else if(_subBlock != nullptr){
            _complexNode->addDependNodeToAllNode(intStartNode);
        }else{
            assert(false);
        }
    }

    Node* SequenceEle::getStateFinishIden() const {
        assert( (_asmNode != nullptr) ^ (_subBlock != nullptr));
        if (_asmNode != nullptr){
            return _stateNode;
        }else if (_subBlock != nullptr){
            return _complexNode->getExitNode();
        }
        assert(false);
    }

    std::vector<Node *> SequenceEle::getEntranceNodes() {
        assert( (_asmNode != nullptr) ^ (_subBlock != nullptr));
        if (_asmNode != nullptr){
            return {_stateNode};
        }else if (_subBlock != nullptr){
            return _complexNode->entranceNodes;
        }
        assert(false);
    }

    bool SequenceEle::isThereForceExitNode() const{
        return (_complexNode != nullptr)  &&
                (_complexNode->isThereForceExitNode());
    }

    Node *SequenceEle::getForceExitNode() const {
        assert(isThereForceExitNode());
        return _complexNode->getForceExitNode();
    }

    bool SequenceEle::isNodeWrap() const{
        return _complexNode != nullptr;
    }

    bool SequenceEle::isBasicNode() const{
        return _stateNode != nullptr;
    }

    NodeWrap *SequenceEle::getNodeWrap() const {
        return _complexNode;
    }

    StateNode *SequenceEle::getBasicNode() const {
        return _stateNode;
    }

    std::string SequenceEle::getDescribe(){

        if (isBasicNode()){
            return _stateNode->getMdIdentVal() + " " + _stateNode->getMdDescribe();
        }else if (isNodeWrap()){
            return _complexNode->getMdIdentVal() + _complexNode->getMdDescribe();
        }
        assert(false);

    }

    void SequenceEle::simulate() const{

        ////////////// simulate state node
        if (_stateNode != nullptr){
            _stateNode->simStartCurCycle();
        }
        ////////////// simulate basic Assignment and subBlock
        if (_asmNode != nullptr){
            _asmNode->simStartCurCycle();
        }
        if (_subBlock != nullptr){
            _subBlock->simStartCurCycle();
        }
    }

    void SequenceEle::finalizeSim() const{
        if (_stateNode != nullptr){
            _stateNode->simExitCurCycle();
        }
        if (_asmNode != nullptr){
            _asmNode->simExitCurCycle();
        }
        if (_subBlock != nullptr){
            _subBlock->simExitCurCycle();
        }
    }

    bool SequenceEle::isCurCycleSimulated() const{

        if (_asmNode != nullptr){
            return _asmNode->isCurValSim();
        }
        if (_subBlock != nullptr){
            return _subBlock->isCurValSim();
        }
        assert(false);
    }

    bool SequenceEle::isBlockOrNodeRunning() const{
        if (_stateNode != nullptr){
            /**do not use asmNode as a trigger*/
            return _stateNode->isBlockOrNodeRunning();
        }
        if (_subBlock != nullptr){
            return _subBlock->isBlockOrNodeRunning();
        }
        assert(false);
    }


    /**
     *
     *
     * sequence flow
     *
     *
     * */

    FlowBlockSeq::FlowBlockSeq(): FlowBlockBase(SEQUENTIAL,
                                                {
                                                        {FLOW_ST_BASE_STACK,
                                                                      FLOW_ST_PATTERN_STACK},
                                                        FLOW_JO_SUB_FLOW,
                                                        true
                                                }),
                                  resultNodeWrap(nullptr) {}

    FlowBlockSeq::~FlowBlockSeq(){
        delete resultNodeWrap;
        for (auto seqEle : _subSeqMetas){
            delete seqEle;
        }
    }

    void FlowBlockSeq::addElementInFlowBlock(Node* node) {
        assert(node != nullptr);
        _subSeqMetas.push_back(new SequenceEle(node, intNodes[INT_RESET]));
        /** base function to notice existence of sub flow element*/
        FlowBlockBase::addElementInFlowBlock(node);
    }

    void FlowBlockSeq::addSubFlowBlock(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        _subSeqMetas.emplace_back(new SequenceEle(subBlock));
        /** base function to notice existence of sub flow block*/
        FlowBlockBase::addSubFlowBlock(subBlock);
    }

    NodeWrap* FlowBlockSeq::sumarizeBlock() {
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockSeq::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockSeq::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockSeq::buildHwComponent() {
        mfAssert(!_subSeqMetas.empty(), "seqBlock has no assignment");
        assert(_conBlocks.empty());
        NodeWrapCycleDet cycleDet;
        /** generate hardware*/
        int idx = 0;
        for (auto& seqMeta: _subSeqMetas) {
            seqMeta->genNode();
            seqMeta->setIdentStateId(getGlobalId(),idx++);
            seqMeta->addToCycleDet(cycleDet);
        }
        /** generate forceExit Node*/
            /***check areThere forceExitNode*/
        std::vector<NodeWrap*> allNw;
        for(auto seqEle: _subSeqMetas){
            if (seqEle->isNodeWrap()){
                allNw.push_back(seqEle->getNodeWrap());
            }
        }
        genSumForceExitNode(allNw);
        /** connect depend node chain*/
        for (size_t idx = 0; (idx+1) < _subSeqMetas.size(); idx++){
            auto lhsNodeWrapper = _subSeqMetas[idx];
            auto rhsNodeWrapper = _subSeqMetas[idx+1];
            rhsNodeWrapper->assignDependDent(lhsNodeWrapper);
        }
        if (isThereIntStart()){
            _subSeqMetas[0]->assignIntStart(intNodes[INT_START]);
        }

        /** build new result NodeWrap*/
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->addEntraceNodes((*_subSeqMetas.begin())->getEntranceNodes());
        resultNodeWrap->addExitNode((*_subSeqMetas.rbegin())->getStateFinishIden());
        resultNodeWrap->setCycleUsed(cycleDet.getCycleVertical());
        if (_areThereForceExit)
            resultNodeWrap->addForceExitNode(_forceExitNode);

    }



    std::string FlowBlockSeq::getMdDescribe() {

        std::string ret;
        int eleCnt = 0;
        ret += "[ " + FlowBlockBase::getMdIdentVal() + " ]\n";
        for (auto seqEle: _subSeqMetas){
            ret += seqEle->getDescribe() + "\n";
            eleCnt++;
        }
        ret += getMdDescribeRecur();
        return ret;

    }

    void FlowBlockSeq::addMdLog(MdLogVal *mdLogVal) {

        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + " ]");
        for (auto seqEle: _subSeqMetas){
            mdLogVal->addVal(seqEle->getDescribe());
        }
        if (resultNodeWrap->isThereForceExitNode()){
            mdLogVal->addVal("forceExit is " + resultNodeWrap->getForceExitNode()->getMdIdentVal() +
                                                   "  " +
                                                   resultNodeWrap->getForceExitNode()->getMdDescribe());
        }

        addMdLogRecur(mdLogVal);

    }

    void FlowBlockSeq::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockSeq::doPostFunction() {
        onDetachBlock();
    }

    /** override flow block simulation*/

    void FlowBlockSeq::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        bool isRunning = false;
        /** simulate each element*/
        for (auto subSeqMeta: _subSeqMetas){
            subSeqMeta->simulate();
            isRunning |= subSeqMeta->isBlockOrNodeRunning();
        }
        if (isRunning) {
            setBlockOrNodeRunning();
            incEngine();
        }
    }


    void FlowBlockSeq::simExitCurCycle(){
        unSetSimStatus();
        unsetBlockOrNodeRunning();
        for(auto subSeqMeta: _subSeqMetas){
            subSeqMeta->finalizeSim();
        }
    }



}
