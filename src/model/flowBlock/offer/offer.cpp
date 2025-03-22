//
// Created by tanawin on 18/3/2025.
//
#include <utility>

#include "model/controller/controller.h"
#include "util/termColor/termColor.h"
#include "offer.h"

namespace kathryn{

    /**
     * Offer block
     */

    FlowBlockOfferBase::FlowBlockOfferBase(std::string  offerName):
    FlowBlockBase(OFFER,
        {
            {FLOW_ST_BASE_STACK},
            FLOW_JO_SUB_FLOW,
            true
        }),
    _offerName(std::move(offerName)){}

    FlowBlockOfferBase::~FlowBlockOfferBase(){
        delete _resultNodeWrap;
    }

    void FlowBlockOfferBase::addElementInFlowBlock(Node* node){
        mfAssert(false, "cannot have raw assignment in offer block");
    }

    void FlowBlockOfferBase::addSubFlowBlock(FlowBlockBase* subBlock){
        assert(subBlock != nullptr);
        mfAssert(subBlock->getFlowType() == OFFER_CHOICE, "the subblock of offerBase: " + _offerName + " must be offer_choice Block");
        auto* ofcBlock = dynamic_cast<FlowBlockOfferChoice*>(subBlock);
        _offerChoiceSubBlocks.push_back(ofcBlock);
        ///// re routing to abandon because we don't want to build hardware component
        FlowBlockBase::addAbandonFlowBlock(subBlock);
    }

    void FlowBlockOfferBase::addConFlowBlock(FlowBlockBase* conBlock){
        mfAssert(false, "cannot add con block in offer block " + _offerName);
    }

    void FlowBlockOfferBase::addAbandonFlowBlock(FlowBlockBase* abandonBlock){
        mfAssert(false, "cannot add abandon block in offer block " + _offerName);
    }

    NodeWrap* FlowBlockOfferBase::sumarizeBlock(){
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }


    void FlowBlockOfferBase::onAttachBlock(){
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockOfferBase::onDetachBlock(){
        ctrl->on_detach_flowBlock(this);
    }

    expression* FlowBlockOfferBase::addLogic(expression* src0, Operable* src1, LOGIC_OP lop){
        assert(src0 != nullptr);

        if (src1 == nullptr){
            return src0;
        }
        switch (lop){
            case BITWISE_AND : {return &((*src0)&(*src1));}
            case BITWISE_OR  : {return &((*src0)|(*src1));}
            default          : {assert(false);}
        }
    }


    void FlowBlockOfferBase::buildHwComponent(){

        assert(_conBlocks.empty());
        assert(_subBlocks.empty());

        /////// we must get all of the offer choice structure
        for (FlowBlockOfferChoice* ofcBlock: _offerChoiceSubBlocks){
            assert(ofcBlock != nullptr); ///// generate the basic structure for the system
            _generatedofcStruct.push_back(ofcBlock->genOfferCSt(_offerName));
        }

        /////// init all node
        _stateNode = new StateNode();

        fillIntResetToNodeIfThere(_stateNode);
        if (isThereIntStart()){
            _stateNode->addDependNode(intNodes[INT_START], nullptr);
        }

        /////// add system Node
        addSysNode(_stateNode);
        /////// assignNode
        /////////// in this case stateNode is not assign because we sent it to outter block

        /////// GENERATE SIGNAL FOR EACH OFFER
        for (int i = 0; i < _generatedofcStruct.size(); i++){
            OfferCSt*        curChoice  = &_generatedofcStruct[i];
            OfferSignalMeta* curSignal  = &curChoice->_offerSignals;
            bool             isTherePrev = (i != 0);
            OfferCSt*        prevChoice = isTherePrev ? &_generatedofcStruct[i-1]  : nullptr;
            OfferSignalMeta* prevSignal = isTherePrev ? &prevChoice->_offerSignals : nullptr;
            //////// assign offer
            if (isTherePrev){ ///// we use * because IT IS INTERMEDIATE SIGNAL
                assert(prevSignal->_isAnyPass != nullptr);
                (*curSignal->_offerSignal) = (*_stateNode->getExitOpr()) & (~(*prevSignal->_isAnyPass));
            }else{
                (*curSignal->_offerSignal) = (*_stateNode->getExitOpr());
            }
            /////// set offer result
            curSignal->_offerResult = addLogic(curSignal->_offerSignal, curSignal->_accept, BITWISE_AND);
            curSignal->_offerResult = addLogic(curSignal->_offerResult, curSignal->_userCond, BITWISE_AND);
            if (curSignal->_accept == nullptr){
                std::cout << TC_YELLOW <<" [WARN] offer: " <<
                _offerName << " -> choice: " << curChoice->_offerChoiceName <<
                " have empty accept signal" << TC_DEF << std::endl;
            }
            //////// set prevPass
            Operable* backPass    = isTherePrev ? prevSignal->_isAnyPass : nullptr;
            curSignal->_isAnyPass = addLogic(curSignal->_offerResult, backPass, BITWISE_OR);

            //////// set log
            curSignal->setOfferRetrievedByBase();

        }

        for (int i = 0; i < _generatedofcStruct.size(); i++){
            OfferCSt*        curChoice  = &_generatedofcStruct[i];
            OfferSignalMeta* curSignal  = &curChoice->_offerSignals;
            /////// assign to basic node
            for (Node* nd: curChoice->_basicNodes){
                assert(nd->getNodeType() == ASM_NODE);
                auto* asmNode = (AsmNode*)nd;
                asmNode->addPreCondition(curSignal->_offerResult, BITWISE_AND);
                _stateNode->addSlaveAsmNode(asmNode);
            }
        }

        /////// build result Node Wrap
        _resultNodeWrap = new NodeWrap();
        _resultNodeWrap->addEntraceNode(_stateNode);
        _resultNodeWrap->addExitNode   (_stateNode);
        _resultNodeWrap->setCycleUsed  (1);

    }

    void FlowBlockOfferBase::addMdLog(MdLogVal* mdLogVal){
        ///// wait for pool
    }

    void FlowBlockOfferBase::doPreFunction(){
        onAttachBlock();
    }

    void FlowBlockOfferBase::doPostFunction(){
        onDetachBlock();
    }







}
