//
// Created by tanawin on 18/3/2025.
//

#ifndef src_model_flowBlock_offer_OFFER_H
#define src_model_flowBlock_offer_OFFER_H
#include "offerChoice.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"


#define offer(offerName) for(auto kathrynBlock = new FlowBlockOfferBase(offerName); kathrynBlock->doPrePostFunction(); kathrynBlock->step())


namespace kathryn{

    const char OFFER_UNNAME[] = "OFFER_UNNAMED";

    class FlowBlockOfferBase: public FlowBlockBase, public LoopStMacro{

    protected:
        ////////// meta
        const std::string       _offerName      = OFFER_UNNAME;

        /////// the offer block
        std::vector<FlowBlockOfferChoice*>
                                _offerChoiceSubBlocks;
        std::vector<OfferCSt>   _generatedofcStruct;
        ////// it ensure the order and priority of the offerchoice

        NodeWrap*         _resultNodeWrap = nullptr;
        StateNode*        _stateNode      = nullptr;

    public:

        explicit FlowBlockOfferBase(std::string  offerName);
        ~FlowBlockOfferBase() override;

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* conBlock) override;
        void addAbandonFlowBlock(FlowBlockBase* abandonBlock) override;
        NodeWrap* sumarizeBlock() override;

        [[nodiscard]]
        std::string getOfferName() const {return _offerName;}

        expression* addLogic(expression* src0, Operable* src1, LOGIC_OP);

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


#endif //src_model_flowBlock_offer_OFFER_H
