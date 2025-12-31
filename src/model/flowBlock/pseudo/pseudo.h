//
// Created by tanawin on 1/12/25.
//

#ifndef SRC_MODEL_FLOWBLOCK_DUMMY_DUMMY_H
#define SRC_MODEL_FLOWBLOCK_DUMMY_DUMMY_H
#include "model/flowBlock/abstract/flowBlock_Base.h"


namespace kathryn{
    /**
     * it is used to store the node that is not belong to any flow block
     * to maintain the assign order to designers
     */
    class FlowBlockPseudo: public FlowBlockBase{

    private:
        AsmNode* _plainNode = nullptr;

    public:
        explicit FlowBlockPseudo(AsmNode* plainNode);
        ~FlowBlockPseudo() override;

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* fb) override;
        void addIntSignal(INT_TYPE type, Operable* signal) override;
        NodeWrap* sumarizeBlock() override;
        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** get describe*/
        std::string getMdDescribe() override;
        void addMdLog(MdLogVal *mdLogVal) override;
        /** extracted system*/
        std::vector<AsmNode*> extract() override;

    };


}

#endif //SRC_MODEL_FLOWBLOCK_DUMMY_DUMMY_H