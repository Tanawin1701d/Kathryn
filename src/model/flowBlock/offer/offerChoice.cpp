//
// Created by tanawin on 18/3/2025.
//


#include "offerChoice.h"
#include "offerPooler.h"
#include "model/controller/controller.h"



namespace kathryn{










    FlowBlockOfferChoice::FlowBlockOfferChoice(const std::string& choiceName):
    FlowBlockBase(OFFER_CHOICE,
    {
        {FLOW_ST_BASE_STACK},
        FLOW_JO_SUB_FLOW,
        true
    }),
    _offerName(choiceName){}

    FlowBlockOfferChoice::FlowBlockOfferChoice(const std::string& offerChoice, Operable& userCond):
    FlowBlockOfferChoice(offerChoice)
    {
        _userCond = &userCond;
    }

    FlowBlockOfferChoice::~FlowBlockOfferChoice() = default;


    void FlowBlockOfferChoice::addElementInFlowBlock(Node* node){
        FlowBlockBase::addElementInFlowBlock(node);
    }

    void FlowBlockOfferChoice::addSubFlowBlock(FlowBlockBase* subBlock){

        ///// for the zif zelse, the controller will extract and add by using node datatype
        mfAssert(false, "(as sub)in OfferChoiceBlock, "
                        "it can only have zif-zelif-zelse block "
                        "and normal assignment");
    }

    void FlowBlockOfferChoice::addConFlowBlock(FlowBlockBase* conBlock){
        mfAssert(false, "(as con) in OfferChoiceBlock, "
                        "it can only have zif-zelif-zelse block "
                        "and normal assignment");
    }

    void FlowBlockOfferChoice::addAbandonFlowBlock(FlowBlockBase* abandonBlock){
        FlowBlockBase::addAbandonFlowBlock(abandonBlock);
    }

    NodeWrap* FlowBlockOfferChoice::sumarizeBlock(){
        mfAssert(false, "cannot sumarize offerchoice block, the block may be placed in out side the offer block");
        return nullptr;
    }

    OfferCSt FlowBlockOfferChoice::genOfferCSt(const std::string& offerBaseName){

        OfferPooler* offerPooler = getOfferPooler();
        OfferMeta&   offerMeta   = offerPooler->getOfferMeta(offerBaseName);
        OfferSignalMeta& metaSignal = offerMeta.getChoice(_offerName);
        if (_userCond != nullptr){
            metaSignal.setUserCondSignal(_userCond);
            metaSignal.setUserCondSetByUser();
        }
        return OfferCSt(_offerName, metaSignal, _basicNodes);
    }


    void FlowBlockOfferChoice::onAttachBlock(){
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockOfferChoice::onDetachBlock(){
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockOfferChoice::buildHwComponent(){
        mfAssert(false,
            "cannot build hardware component in offer choice block, the block " +
            _offerName +
            " may be placed in incorrect master block block");
    }

    void FlowBlockOfferChoice::addMdLog(MdLogVal* mdLogVal){
        mdLogVal->addVal("[" + FlowBlockBase::getMdIdentVal() + "]");
        mdLogVal->addVal(" choice Name " + _offerName);
    }

    void FlowBlockOfferChoice::doPreFunction(){
        onAttachBlock();
    }

    void FlowBlockOfferChoice::doPostFunction(){
        onDetachBlock();
    }
}