//
// Created by tanawin on 3/12/2566.
//
#include "controller.h"

namespace kathryn{


    FlowBlockBase* ModelController::getTopFlowBlockBase() {
        if (flowBlockStacks[FLOW_ST_BASE_STACK].empty()){
           return nullptr;
        }
        return flowBlockStacks[FLOW_ST_BASE_STACK].top();
    }

    void ModelController::popFlowBlock(FlowBlockBase* fb){
        assert(!flowBlockStacks[FLOW_ST_BASE_STACK].empty());
        assert( flowBlockStacks[FLOW_ST_BASE_STACK].top() == fb);

        for (int stIdx: fb->getSelFbStack()){
            assert(stIdx < FLOW_ST_CNT);
            assert(!flowBlockStacks[stIdx].empty());
            assert(flowBlockStacks[stIdx].top() == fb);
            logMF(fb, "pop_flowBlock " + std::to_string(stIdx));
            flowBlockStacks[stIdx].pop();
        }
    }

    void ModelController::pushFlowBlock(kathryn::FlowBlockBase *fb) {
        assert(fb != nullptr);
        for (int stIdx: fb->getSelFbStack()){
            logMF(fb, "push to stack " + std::to_string(stIdx));
            assert(stIdx < FLOW_ST_CNT);
            flowBlockStacks[stIdx].push(fb);
        }
    }

    bool ModelController::isAllFlowStackEmpty(){
        bool emptyStatus = true;
        for(const auto & flowBlockStack : flowBlockStacks){
            emptyStatus &= flowBlockStack.empty();
        }
        return emptyStatus;
    }

    void ModelController::purifyFlowStack() {
        FlowBlockBase* fb = getTopFlowBlockBase();
        if (fb == nullptr){return;}
        if (fb->isLazyDelete()){
            fb->unsetLazyDelete();
            logMF(fb, "strong purify stack");
            on_detach_flowBlock(fb);
        }
    }

    void ModelController::on_attach_flowBlock(FlowBlockBase *fb) {
        /*** check purify flow stack*/
        logMF(fb, "on_attach_flowBlock");
        assert(fb != nullptr);
        if (fb->getPurifyReq()){
            logMF(fb, "try purify stack");
            purifyFlowStack();
        }
        /*** add to stack*/
        pushFlowBlock(fb);
    }

    void ModelController::on_detach_flowBlock(FlowBlockBase* fb) {
        assert(fb != nullptr);
        logMF(fb, "on_detach_flowBlock");
        /***if it is lazy delete do not delete it*/
        if (fb->isLazyDelete()){
            logMF(fb, "on_detach_flowBlock and not delete due to lazy delete");
            return;
        }
        /**get top of the flow block base and build the hardware*/
        FlowBlockBase* topFb = getTopFlowBlockBase();
        assert(fb == topFb);
        popFlowBlock(topFb);
        topFb->buildHwComponent();


        /**get front node to inject the subblock*/
        FlowBlockBase* frontFb = getTopFlowBlockBase();
        if (frontFb == nullptr){
            logMF(fb, "addFlowBlock to module");
            Module* parentMod = getTargetModulePtr();
            parentMod->addFlowBlock(topFb);
        }else if (frontFb->getJoinFbPol() == FLOW_JO_CON_FLOW){
            /**it is consecutive block*/
            logMF(fb, "addFlowBlock to be con module");
            frontFb->addConFlowBlock(topFb);
        }else if (frontFb->getJoinFbPol() == FLOW_JO_SUB_FLOW){
            /**it is sub block*/
            logMF(fb, "addFlowBlock to be sub module");
            frontFb->addSubFlowBlock(topFb);
        }else{
            assert(false);
        }

    }

    FLOW_BLOCK_TYPE ModelController::get_top_pattern_flow_block_type(){


        if (flowBlockStacks[FLOW_ST_PATTERN_STACK].empty()){
            return DUMMY_BLOCK;
        }else{
            FlowBlockBase* fb = flowBlockStacks[FLOW_ST_PATTERN_STACK].top();

            assert(fb != nullptr);
            FLOW_BLOCK_TYPE fbType = fb->getFlowType();
            assert(fbType >= SEQUENTIAL && fbType <= PARALLEL_AUTO_SYNC);
            return fbType;
        }

    }




}