//
// Created by tanawin on 1/12/25.
//

#ifndef SRC_MODEL_FLOWBLOCK_DUMMY_DUMMY_H
#define SRC_MODEL_FLOWBLOCK_DUMMY_DUMMY_H
#include "model/flow_block/abstract/flow_block__base.h"


namespace kathryn{
    /**
     * it is used to store the node that is not belong to any flow block
     * to maintain the assign order to designers
     */
    class FlowBlockPseudo: public FlowBlockBase{

    private:
        AsmNode* _plainNode = nullptr;

    public:
        explicit FlowBlockPseudo(AsmNode* plain_node);
        ~FlowBlockPseudo() override;

        /** for controller add the local element to this sub block*/
        void add_element_in_flow_block(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* sub_block) override;
        void add_con_flow_block(FlowBlockBase* fb) override;
        void add_int_signal(INT_TYPE type, Operable* signal) override;
        NodeWrap* sumarize_block() override;
        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_component() override;
        /** get describe*/
        std::string get_md_describe() override;
        void add_md_log(MdLogVal *md_log_val) override;
        /** extracted system*/
        std::vector<AsmNode*> extract() override;

    };


}

#endif //SRC_MODEL_FLOWBLOCK_DUMMY_DUMMY_H