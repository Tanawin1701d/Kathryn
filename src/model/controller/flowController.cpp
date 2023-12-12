//
// Created by tanawin on 3/12/2566.
//
#include "controller.h"
namespace kathryn{

    FlowBlockBase* Controller::getTopFlowBlock(){
        assert(!flowBlockStack.empty());
        return flowBlockStack.top();
    }

    void Controller::tryPopFbBaseFromStack(FlowBlockBase* flowPtr,
                                           std::stack<FlowBlockBase*>& srcSt) {
        /**skip when empty skip when not match*/
        if (srcSt.empty() || (srcSt.top() != flowPtr))
            return;
        srcSt.pop();

    }

    void Controller::tryPopFbIfFromStack(kathryn::FlowBlockBase *flowPtr,
                                         std::stack<FlowBlockIf *> &srcSt) {
        if (srcSt.empty() || (srcSt.top() != flowPtr))
            return;
        srcSt.pop();
    }

    void Controller::tryPopCtrlFlowFromAllStack(FlowBlockBase* flowPtr){
        tryPopFbBaseFromStack(flowPtr, flowBlockStack);
        tryPopFbBaseFromStack(flowPtr, patternFlowBlockStack);
        tryPopFbIfFromStack(flowPtr, ifBlockStack);
    }


    void Controller::removeLazyFbFromTopStack() {
        if (flowBlockStack.empty()){
            return;
        }

        if (flowBlockStack.top()->isLazyDelete()){
            auto ifFlowBlock = ifBlockStack.top();
            assert(ifFlowBlock != nullptr);
            /** build hardware component */
            ifFlowBlock->buildHwComponent();
            /** try pop from stack*/
            tryPopCtrlFlowFromAllStack(flowBlockStack.top());
            /** addElement to master of this block */
            if (!flowBlockStack.empty()){
                getTopFlowBlock()->addSubFlowBlock(ifFlowBlock);
            }
        }
    }


    void Controller::purifyFlowStack() {
        removeLazyFbFromTopStack();
    }


    /** use for any flow block except condition block*/
    void Controller::on_attach_flowBlock(FlowBlockBase* fb) {

        assert(fb != nullptr);
        assert(fb->getFlowType() != IF);
        assert(fb->getFlowType() != ELIF);
        assert(fb->getFlowType() != ELSE);
        purifyFlowStack();

        /** we must save head of flow block for deleting and debugging*/
        if (flowBlockStack.empty()){
            getTargetModuleEle().md->addFlowBlock(fb);
        }

        flowBlockStack.push(fb);

         /** push to stack in each case*/
        FLOW_BLOCK_TYPE flowType = fb->getFlowType();
         switch (flowType){
             case SEQUENTIAL :
             case PARALLEL   :
                 patternFlowBlockStack.push(fb);
                 break;
         }

    }

    void Controller::on_detach_flowBlock(FlowBlockBase* fb) {
        assert(fb != nullptr);
        assert(fb->getFlowType() != IF);
        assert(fb->getFlowType() != ELIF);
        assert(fb->getFlowType() != ELSE);
        purifyFlowStack();
        auto top= getTopFlowBlock();
        assert(top == fb);

        /**we must build hardware component because sub element require module position*/
        fb->buildHwComponent();

        tryPopCtrlFlowFromAllStack(fb);
        if (!flowBlockStack.empty()){
            getTopFlowBlock()->addSubFlowBlock(fb);
        }
    }

    void Controller::on_attach_flowBlock_if(FlowBlockIf *fb) {
        assert(fb != nullptr);
        assert(fb->getFlowType() == IF);
        assert(!flowBlockStack.empty());
        purifyFlowStack();
        flowBlockStack.push(fb);
        ifBlockStack.push(fb);

    }

    void Controller::on_attach_flowBlock_elif(FlowBlockElif *fb) {
        assert(fb != nullptr);
        assert(fb->getFlowType() == ELIF || fb->getFlowType() == ELSE);
        assert(!flowBlockStack.empty());
        /*** do not purify flow stack*/
        flowBlockStack.push(fb);
    }

    void Controller::on_detach_flowBlock_elif(FlowBlockElif *fb) {
        assert(fb != nullptr);
        assert(fb->getFlowType() == ELIF || fb->getFlowType() == ELSE);
        assert(!flowBlockStack.empty());
        assert(!ifBlockStack.empty());
        assert(fb == getTopFlowBlock());
        /**build hardware component*/
        fb->buildHwComponent();
        /***pop this block from hardware stach*/
        tryPopCtrlFlowFromAllStack(fb);
        /** transfer data to master if block*/
        ifBlockStack.top()->addElifNodeWrap(fb);
    }

    FLOW_BLOCK_TYPE Controller::get_top_pattern_flow_block_type(){
        if (patternFlowBlockStack.empty()){
            return DUMMY_BLOCK;
        }
        return patternFlowBlockStack.top()->getFlowType();
    }





}