//
// Created by tanawin on 10/1/26.
//

#ifndef MODEL_FLOWBLOCK_STATE_ZCASE_H
#define MODEL_FLOWBLOCK_STATE_ZCASE_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"

#define zcase(caseValue) for(auto kathrynBlock = new FlowBlockZCase(caseValue); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define zcasedef for(auto kathrynBlock = new FlowBlockZCase(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    class FlowBlockZCase: public FlowBlockBase, public LoopStMacro{
    private:
        int _caseValue;
        std::vector<ClassAssignMeta*> _assignMetas;

    public:
        explicit FlowBlockZCase(int caseValue = -1);
        ~FlowBlockZCase() override;

        /** for controller add the local element to this sub block*/
        void add_basic_node(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* subBlock) override;
        void add_con_flow_block(FlowBlockBase* fb) override;
        void add_intr_signal(INT_TYPE type, Operable* signal) override;
        NodeWrap* sumarize_block() override;
        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_component() override;
        /** get describe*/
        std::string get_md_describe() override;
        void add_md_log(MdLogVal *mdLogVal) override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

        std::vector<ClassAssignMeta*> getClassAssMetas();
        int getCaseId() const;




    };

}

#endif //SRC_MODEL_FLOWBLOCK_STATE_ZCASE_H