//
// Created by tanawin on 2/12/2566.
//

#include "flowBlock_Base.h"

#include <utility>
#include "model/controller/controller.h"
#include "model/flowBlock/seq/seq.h"
#include "model/flowBlock/par/par.h"

namespace kathryn{

    int nextFbIdx = 0;


    FlowBlockBase::FlowBlockBase(FLOW_BLOCK_TYPE type, FB_CTRL_COM_META fbCtrlComMeta):
            FlowIdentifiable(FBT_to_string(type)),
            /** flow element*/
            _type(type),
            ctrl(getControllerPtr()),
            lazyDeletedRequired(false),
            _fbId(nextFbIdx++),
            /** controller communication policy*/
            _fbCtrlComMeta(std::move(fbCtrlComMeta)),
            /** exit management*/
            _areThereForceExit(false),
            _forceExitNode(nullptr),
            _flowSimEngine(new FlowBaseSimEngine(this))
    {
                /**initialize interrupt node*/
                for (int intType = 0; intType < INT_CNT; intType++){
                    intNodes[intType] = nullptr;
                }
    }

    FlowBlockBase::~FlowBlockBase(){
        for (auto basicNode : _basicNodes){
            delete basicNode;
        }
        for (auto sub_fb: _subBlocks){
            delete sub_fb;
        }
        for (auto con_fb: _conBlocks){
            delete con_fb;
        }
        for (auto abandon_fb:_abandonedBlocks){
            delete abandon_fb;
        }
        for (auto intNode: intNodes){
            delete intNode;
        }
        delete holdNode;

        delete _forceExitNode;
        delete _flowSimEngine;
        /////// it is safe to delete nullptr
    }

    FlowBlockBase* FlowBlockBase::genImplicitSubBlk(FLOW_BLOCK_TYPE defaultType) {
        /** determine next flow block*/
        FLOW_BLOCK_TYPE nextFbType = ctrl->get_top_pattern_flow_block_type();
        if (nextFbType == DUMMY_BLOCK){
            nextFbType = defaultType;
        }
        /** create subblock*/
        if ( (nextFbType == PARALLEL_NO_SYN) || (nextFbType == PARALLEL_AUTO_SYNC)){
            return new FlowBlockParAuto();
        }else if (nextFbType == SEQUENTIAL){
            return new FlowBlockSeq();
        }else{
            assert(false); /** can't determine flow type*/
        }
    }

    void FlowBlockBase::genSumForceExitNode(std::vector<NodeWrap *> &nws) {
        /** check that there is force exit node*/
        for (auto nw : nws){
            _areThereForceExit |= (nw->getForceExitNode() != nullptr);
        }

        /** build pseudo node*/
        if (_areThereForceExit){
            _forceExitNode = new PseudoNode(1, BITWISE_OR);
            for (auto nw : nws){
                if (nw->getForceExitNode() != nullptr){
                    _forceExitNode->addDependNode(nw->getForceExitNode(), nullptr);
                }
            }
            _forceExitNode->assign();
        }
    }

    void FlowBlockBase::fillIntRstSignalToChild(){
        for (auto subBlockPtr: _subBlocks){
            assert(subBlockPtr != nullptr);
            for (auto signal: intSignals[INT_RESET]){
                subBlockPtr->addIntSignal(INT_RESET, signal);
            }

        }

        for (auto conBlockPtr: _conBlocks){
            assert(conBlockPtr != nullptr);
            for (auto signal: intSignals[INT_RESET]){
                conBlockPtr->addIntSignal(INT_RESET, signal);
            }
        }
    }

    void FlowBlockBase::genIntNode(){

        for (int intType = 0; intType < INT_CNT; intType++){
            if (intSignals[intType].empty()){
                continue;
            }
            /* init node **/
            intNodes[intType] = new OprNode(intSignals[intType][0]);

            for (int sigId = 1; sigId < intSignals[intType].size(); sigId++){
                intNodes[intType]->addLogic(
                    intNodes[intType]->_value,
                    intSignals[intType][sigId],
                    BITWISE_OR
                );
            }
        }
    }

    Operable* FlowBlockBase::genIntSumSignal(bool isAndCond, INT_TYPE intrType){

        std::vector<Operable*>& targetSignals = intSignals[intrType];

        if (targetSignals.empty()){
            return nullptr;
        }
        Operable* resultSignal = targetSignals[0];
        for (int idx = 1; idx < static_cast<int>(targetSignals.size()); idx++){
            mfAssert(targetSignals[idx]->getOperableSlice().getSize() == 1, "interrupt signal size cannot more than one");

            resultSignal = isAndCond ? &((*resultSignal) & (*targetSignals[idx]))
                                     : &((*resultSignal) | (*targetSignals[idx]));
        }
        return resultSignal;

    }

    bool FlowBlockBase::isThereIntStart(){
        return intNodes[INT_START] != nullptr;
    }

    bool FlowBlockBase::isThereIntRst(){
        return intNodes[INT_RESET] != nullptr;
    }

    void FlowBlockBase::fillHoldSignalToChild(){
        for (auto subBlockPtr: _subBlocks){
            assert(subBlockPtr != nullptr);
            for (auto signal: holdSignals){
                subBlockPtr->addHoldSignal(signal);
            }
        }

        for (auto conBlockPtr: _conBlocks){
            assert(conBlockPtr != nullptr);
            for (auto signal: holdSignals){
                conBlockPtr->addHoldSignal(signal);
            }
        }
    }

    void FlowBlockBase::genHoldNode(){

        if (holdSignals.empty()){
            return;
        }
        holdNode = new OprNode(holdSignals[0]);

        for (int sigId = 1; sigId < static_cast<int>(holdSignals.size()); sigId++){
            holdNode->addLogic(
                holdNode->_value,
                holdSignals[sigId],
                BITWISE_OR
            );
        }

    }

    bool FlowBlockBase::isThereHold(){
        return holdNode  != nullptr;
    }

    void FlowBlockBase::buildHwMaster(){
        /** pass the Int reset signal and holding signal to child block*/
        /** dont fill interrupt start signal because this block will start it*/
        fillIntRstSignalToChild();
        fillHoldSignalToChild();

        /** build the sub block first*/
        buildSubHwComponent();

        /** we so sure now that all sub  Block is ready*/
        /** start build the node for int reset start and hold signal*/
        genIntNode();
        genHoldNode();

        buildHwComponent();
    }

    void FlowBlockBase::buildSubHwComponent(){

        for (auto subBlockPtr: _subBlocks){
            assert(subBlockPtr != nullptr);
            subBlockPtr->buildHwMaster();
        }

        for (auto conBlockPtr: _conBlocks){
            assert(conBlockPtr != nullptr);
            conBlockPtr->buildHwMaster();
        }

    }

    /**
     * condition such as cif sif celif may receiver condition that has
     * more than 1 in  bitsize, we need to convert it to one in bitsize
     ***/

    Operable* FlowBlockBase::purifyCondition(Operable* rawOpr){

            assert(rawOpr != nullptr);
            Slice rawSl = rawOpr->getOperableSlice();
            assert(rawSl.getSize() != 0);

            if (rawSl.getSize() == 1){
                return rawOpr;
            }
            makeVal(autoCmpCondVal, rawSl.getSize(), 0);
            return &((*rawOpr) > autoCmpCondVal);

    }

    FlowBlockBase* FlowBlockBase::scanMasterJoinSubBlock(){
        //////// the master join block must have only 1 sub block
        FlowBlockBase* resultFb = nullptr;
        for (FlowBlockBase* subFb: _subBlocks){
            mfAssert(subFb != nullptr, "subBlock cannot be nullptr");
            if (subFb->isJoinMaster()){
                mfAssert(resultFb == nullptr, "duplicated master join block");
                resultFb = subFb;
            }
        }
        return resultFb;
    }

    std::vector<FlowBlockBase::sortEle> FlowBlockBase::sortSubAndConFbInOrder() {

        std::vector<sortEle> poolEle;
        /***pool sub block in to the array*/
        for(int i = 0; i < _subBlocks.size(); i++){
            poolEle.push_back({_subBlocks[i], _subBlocksOrder[i]});
        }
        for(int i = 0; i < _conBlocks.size(); i++){
            poolEle.push_back({_conBlocks[i], _conBlocksOrder[i]});
        }
        /**sort array*/
        std::sort(poolEle.begin(), poolEle.end());

        return poolEle;
    }

}