//
// Created by tanawin on 18/3/2025.
//

#ifndef src_model_flowBlock_offer_OFFERCHOICE_H
#define src_model_flowBlock_offer_OFFERCHOICE_H

#include "offerMeta.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"

///// ofc is from offer choice
#define ofc(offerChoiceName) for(auto kathrynBlock = new FlowBlockOfferChoice(offerChoiceName); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define ofcc(offerChoiceName, cond) for(auto kathrynBlock = new FlowBlockOfferChoice(offerChoiceName, cond); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{



    class FlowBlockOfferChoice: public FlowBlockBase, public LoopStMacro{

    protected:
        const std::string _offerName = OFFERCHOICE_UNNAME;
        Operable* _userCond = nullptr;


    public:

        explicit FlowBlockOfferChoice(const std::string& offerChoice);
        explicit FlowBlockOfferChoice(const std::string& offerChoice, Operable& userCond);
        ~FlowBlockOfferChoice() override;


        /** for controller add the local element to this sub block*/
        void      addElementInFlowBlock(Node* node) override;
        void      addSubFlowBlock(FlowBlockBase* subBlock) override;
        void      addConFlowBlock(FlowBlockBase* conBlock) override;
        void      addAbandonFlowBlock(FlowBlockBase* abandonBlock) override;
        NodeWrap* sumarizeBlock() override;

        [[nodiscard]]
        std::string getOfferName() const {return _offerName;}
        [[nodiscard]]
        OfferCSt genOfferCSt(const std::string& offerBaseName);

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** get describe*/
        void addMdLog(MdLogVal* mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

    };

}



#endif //src_model_flowBlock_offer_OFFERCHOICE_H
