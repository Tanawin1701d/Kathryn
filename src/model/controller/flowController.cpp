//
// Created by tanawin on 3/12/2566.
//
#include "controller.h"

namespace kathryn{


    /***
     *
     * flow stack can not have lazy block more than one at a time
     * because if there is consequence lazy block the prior one must be delete
     *
     * */


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

    void ModelController::pushFlowBlock(FlowBlockBase* fb) {
        assert(fb != nullptr);
        for (int stIdx: fb->getSelFbStack()){
            logMF(fb, "push to stack " + std::to_string(stIdx));
            assert(stIdx < FLOW_ST_CNT);
            flowBlockStacks[stIdx].push(fb);
        }
    }

    void ModelController::detachTopFlowBlock() {

        /**get top of the flow block base and build the hardware*/
        FlowBlockBase* topFb = getTopFlowBlockBase();
        assert(topFb != nullptr);
        logMF(topFb, "on_detach_flowBlock");
        popFlowBlock(topFb);
        //////topFb->buildHwComponent();


        /**get front node to inject the subblock*/
        FlowBlockBase* frontFb = getTopFlowBlockBase();
        if (!isTopFbBelongToTopModule()){
            logMF(topFb, "addFlowBlock to module");
            Module* parentMod = getTopModulePtr();
            parentMod->addFlowBlock(topFb);
        }else if (topFb->getJoinFbPol() == FLOW_JO_CON_FLOW){
            /**it is consecutive block*/
            logMF(topFb, "addFlowBlock to be con module");
            frontFb->addConFlowBlock(topFb);
        }else if (topFb->getJoinFbPol() == FLOW_JO_SUB_FLOW){
            /**it is sub block*/
            logMF(topFb, "addFlowBlock to be sub module");
            frontFb->addSubFlowBlock(topFb);
        }else if (topFb->getJoinFbPol() == FLOW_JO_EXT_FLOW){
            logMF(topFb, "extract flowblock and give it to basic asm");
            for (auto basicNode: topFb->getBasicNode()){
                frontFb->addElementInFlowBlock(basicNode);
            }
            /***we must delete this due to*/
            ///// delete topFb;
        }else{
            assert(false);
        }

    }

    void ModelController::assignFlowBlockParent(FlowBlockBase* fb){

        /** assign master module*/
        Module* parentMod = getTargetModuleEle().md;
        assert(parentMod != nullptr);
        fb->setParent(parentMod);

        /** assign master flow block*/
        if (isTopFbBelongToTopModule()){
            FlowBlockBase* topFb = getTopFlowBlockBase();
            fb->setParent(topFb);
        }
    }


    bool ModelController::isAllFlowStackEmpty(){
        bool emptyStatus = true;
        for(const auto & flowBlockStack : flowBlockStacks){
            emptyStatus &= flowBlockStack.empty();
        }
        return emptyStatus;
    }

    bool ModelController::isTopFbBelongToTopModule(){
        assert(getTopModulePtr() != nullptr);
        return (!flowBlockStacks[FLOW_ST_BASE_STACK].empty()) &&
                (flowBlockStacks[FLOW_ST_BASE_STACK].top()->getModuleParent() == getTopModulePtr());

    }

    void ModelController::tryPurifyFlowStack() {
        FlowBlockBase* fb = getTopFlowBlockBase();
        if (fb == nullptr){return;}
        if (fb->isLazyDelete()){
            logMF(fb, "strong purify stack");
            detachTopFlowBlock();
        }
    }

    void ModelController::on_attach_flowBlock(FlowBlockBase *fb) {
        /*** check purify flow stack*/
        logMF(fb, "on_attach_flowBlock");
        assert(fb != nullptr);
        if (fb->getPurifyReq()){
            logMF(fb, "try purify stack");
            tryPurifyFlowStack();
        }

        assignFlowBlockParent(fb);
        fb->assignInheritName();
        /*** add to stack*/
        pushFlowBlock(fb);
    }

    void ModelController::on_detach_flowBlock(FlowBlockBase* fb) {

        /** if current flowblock is lazy delete do not detach it*/
        if (fb->isLazyDelete()){
            return;
        }

        /** there must be at most one flow block that must be detach
         * due to last lazy delete pupose
         * */
        auto topFb = getTopFlowBlockBase();
        if (fb != topFb){
            ////// it must be lazy delete block inside that it is not deleted yet
            assert(topFb->isLazyDelete());
            /////// delete it now
            tryPurifyFlowStack();
        }
        /** get our block detach*/
        auto actualDetachBlock = getTopFlowBlockBase();
        assert(actualDetachBlock == fb);
        detachTopFlowBlock();

    }

    FLOW_BLOCK_TYPE ModelController::get_top_pattern_flow_block_type(){

        bool topPatternFbBelongToTopModule =
                  (!flowBlockStacks[FLOW_ST_PATTERN_STACK].empty())
                && (flowBlockStacks[FLOW_ST_PATTERN_STACK].top()->getModuleParent() == getTopModulePtr()
                );

        if (topPatternFbBelongToTopModule){
            FlowBlockBase* fb = flowBlockStacks[FLOW_ST_PATTERN_STACK].top();
            assert(fb != nullptr);
            FLOW_BLOCK_TYPE fbType = fb->getFlowType();
            assert(fbType >= SEQUENTIAL && fbType <= PARALLEL_AUTO_SYNC);
            return fbType;
        }else{
            return DUMMY_BLOCK;
        }

    }




}