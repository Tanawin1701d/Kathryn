//
// Created by tanawin on 8/12/2566.
//

#ifndef KATHRYN_ELIF_H
#define KATHRYN_ELIF_H
#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"

#define celif(expr) for(auto kathryn_block = new FlowBlockElif(expr); kathryn_block->do_pre_post_function(); kathryn_block->step())
#define selif(expr) for(auto kathryn_block = new FlowBlockElif(expr); kathryn_block->do_pre_post_function(); kathryn_block->step())
#define celse       for(auto kathryn_block = new FlowBlockElif();     kathryn_block->do_pre_post_function(); kathryn_block->step())
#define selse       for(auto kathryn_block = new FlowBlockElif();     kathryn_block->do_pre_post_function(); kathryn_block->step())

namespace kathryn{

    class FlowBlockElif: public FlowBlockBase, public LoopStMacro{
    private:
        FlowBlockBase* implicitSubBlock = nullptr;
        NodeWrap* resultNodeWrapper = nullptr;
        Operable* _cond = nullptr;

    public:
        explicit FlowBlockElif(Operable& cond);
        explicit FlowBlockElif();

        ~FlowBlockElif() override;

        /** for controller add the local element to this sub block*/
        void add_basic_node(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* subBlock) override;
        NodeWrap* sumarize_block() override;
        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_component() override;

        std::string get_md_describe() override;
        void add_md_log(MdLogVal *mdLogVal) override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

        [[nodiscard]]
        Operable* getCondition() const{
            return _cond;
        }
    };


}

#endif //KATHRYN_ELIF_H
