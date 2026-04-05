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


    FlowBlockBase* ModelController::getTopFlowBlockBasePtr() {
        if (flowBlockStacks[FLOW_ST_BASE_STACK].empty()){
           return nullptr;
        }
        return flowBlockStacks[FLOW_ST_BASE_STACK].top();
    }

    FlowBlockBase* ModelController::getTopFlowBlockBasePtr(FLOW_STACK_TYPE flowStackType){
        assert(flowStackType >= 0);
        assert(flowStackType < FLOW_ST_CNT);
        if (flowBlockStacks[flowStackType].empty()){
            return nullptr;
        }
        return flowBlockStacks[flowStackType].top();
    }

    void ModelController::popFlowBlock(FlowBlockBase* fb){
        assert(!flowBlockStacks[FLOW_ST_BASE_STACK].empty());
        assert( flowBlockStacks[FLOW_ST_BASE_STACK].top() == fb);

        for (int stIdx: fb->getSelFbStack()){
            assert(stIdx < FLOW_ST_CNT);
            assert(!flowBlockStacks[stIdx].empty());
            assert(flowBlockStacks[stIdx].top() == fb);
            logMf(fb, "pop_flowBlock " + std::toString(stIdx));
            flowBlockStacks[stIdx].pop();
        }
    }

    void ModelController::pushFlowBlock(FlowBlockBase* fb) {
        assert(fb != nullptr);
        for (int stIdx: fb->getSelFbStack()){
            logMf(fb, "push to stack " + std::toString(stIdx));
            assert(stIdx < FLOW_ST_CNT);
            flowBlockStacks[stIdx].push(fb);
        }
    }

    void ModelController::detachTopFlowBlock() {

        /**get top of the flow block base and build the hardware*/
        FlowBlockBase* topFb = getTopFlowBlockBasePtr();
        assert(topFb != nullptr);
        logMf(topFb, "on_detach_flowBlock");
        popFlowBlock(topFb);
        //////topFb->buildHwComponent();


        /**get front node to inject the subblock*/
        FlowBlockBase* frontFb = getTopFlowBlockBasePtr();
        if (!isTopFbBelongToTopModule()){
            logMf(topFb, "addFlowBlock to module");
            Module* parentMod = getTopModulePtr();
            parentMod->addFlowBlock(topFb);
        }else if (topFb->getJoinFbPol() == FLOW_JO_CON_FLOW){
            /**it is consecutive block*/
            logMf(topFb, "addFlowBlock to be con module");
            frontFb->addConFlowBlock(topFb);
        }else if (topFb->getJoinFbPol() == FLOW_JO_SUB_FLOW){
            /**it is sub block*/
            logMf(topFb, "addFlowBlock to be sub module");
            frontFb->addSubFlowBlock(topFb);
        }else if (topFb->getJoinFbPol() == FLOW_JO_EXT_FLOW){
            logMf(topFb, "extract flowblock and give it to basic asm");
            std::vector<AsmNode*> extractedAsmNode = topFb->extract();
            for (auto asmNode: extractedAsmNode){
                frontFb->addElementInFlowBlock(asmNode);
            }
            // for (auto basicNode: topFb->getBasicNode()){
            //     frontFb->addElementInFlowBlock(basicNode);
            // }
            frontFb->addAbandonFlowBlock(topFb);
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
            FlowBlockBase* topFb = getTopFlowBlockBasePtr();
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

    bool ModelController::isFlowStackEmpty(FLOW_STACK_TYPE flowStackType){

        assert(flowStackType >= 0);
        assert(flowStackType < FLOW_ST_CNT);

        return flowBlockStacks[flowStackType].empty();
    }


    bool ModelController::isTopFbBelongToTopModule(){
        assert(getTopModulePtr() != nullptr);
        return (!flowBlockStacks[FLOW_ST_BASE_STACK].empty()) &&
                (flowBlockStacks[FLOW_ST_BASE_STACK].top()->getModuleParentPtr() == getTopModulePtr());

    }

    void ModelController::tryPurifyFlowStack() {
        FlowBlockBase* fb = getTopFlowBlockBasePtr();
        if (fb == nullptr){return;}
        if (fb->isLazyDelete()){
            logMf(fb, "strong purify stack");
            detachTopFlowBlock();
        }
    }

    void ModelController::on_attach_flowBlock(FlowBlockBase *fb) {
        /*** check purify flow stack*/
        logMf(fb, "on_attach_flowBlock");
        assert(fb != nullptr);
        if (fb->getPurifyReq()){
            logMf(fb, "try purify stack");
            tryPurifyFlowStack();
        }

        assignFlowBlockParent(fb);
        fb->buildInheritName();
        /*** add to stack*/
        pushFlowBlock(fb);
    }

    void ModelController::on_detach_flowBlock(FlowBlockBase* fb) {

        /** to clean lazy delete of the flow block
         * due to last lazy delete pupose
         * */
        auto topFb = getTopFlowBlockBasePtr();
        if (fb != topFb){
            ////// it must be lazy delete block inside that it is not deleted yet
            assert(topFb->isLazyDelete());
            /////// delete it now
            tryPurifyFlowStack();
        }

        /** get our block detach*/
        auto actualDetachBlock = getTopFlowBlockBasePtr();

        /** if current flowblock is lazy delete do not detach it*/
        assert(actualDetachBlock == fb);
        if (fb->isLazyDelete()){
            return;
        }else{
            detachTopFlowBlock();
        }

    }

    void ModelController::on_attachAndDetach_intrSignal(INT_TYPE intType, Operable* sig) {
        assert(sig != nullptr);
        tryPurifyFlowStack();
        mfAssert(isTopFbBelongToTopModule(), "There is no flow block to add intr signal");
        auto topFb = getTopFlowBlockBasePtr();
        topFb->addIntSignal(intType, sig);

    }


    // Operable& ModelController::onGetCheckNextPipblkReadySignal() {
    //     tryPurifyFlowStack();
    //     mfAssert(isTopFbBelongToTopModule(), "there is no flow block to get next ready signal");
    //     assert((!flowBlockStacks[FLOW_ST_PIP_WRAP].empty()) &&
    //            (flowBlockStacks[FLOW_ST_PIP_WRAP].top()->getModuleParentPtr() == getTopModulePtr())
    //            );
    //
    //     /*** get and gen signal*/
    //     auto pipWrapBlk = flowBlockStacks[FLOW_ST_PIP_WRAP].top();
    //     assert(pipWrapBlk->getFlowType() == PIPE_WRAPPER);
    //     auto castedpipWrapBlk = (FlowBlockPipeWrapper*) pipWrapBlk;
    //     return castedpipWrapBlk->getNextPipBlockReadySignal();
    // }

    FLOW_BLOCK_TYPE ModelController::getTopPatternFlowBlockType(){

        bool topPatternFbBelongToTopModule =
                  (!flowBlockStacks[FLOW_ST_PATTERN_STACK].empty())
                && (flowBlockStacks[FLOW_ST_PATTERN_STACK].top()->getModuleParentPtr() == getTopModulePtr()
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

    bool ModelController::isTopOfStackBelongToTheSameModule(FLOW_STACK_TYPE a,
                                                            FLOW_STACK_TYPE b){

        if (isFlowStackEmpty(a) ||
            isFlowStackEmpty(b)){return false;}


        FlowBlockBase* flowBlockA = getTopFlowBlockBasePtr(a);
        FlowBlockBase* flowBlockB = getTopFlowBlockBasePtr(b);
        assert(flowBlockA != nullptr && flowBlockB != nullptr);

        Module* parentA = flowBlockA->getModuleParentPtr();
        Module* parentB = flowBlockB->getModuleParentPtr();

        assert(parentA != nullptr &&  parentB != nullptr);
        return parentA == parentB;

    }





}