//
// Created by tanawin on 3/12/2566.
//
#include "controller.h"

#include "util/logger/logger.h"
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
        /** debug */

    }

    void Controller::tryPopFbIfFromStack(kathryn::FlowBlockBase *flowPtr,
                                         std::stack<FlowBlockIf *> &srcSt) {
        if (srcSt.empty() || (srcSt.top() != flowPtr))
            return;
        srcSt.pop();
    }

    void Controller::tryPopCtrlFlowFromAllStack(FlowBlockBase* flowPtr){
        /** debug*/
        logMF(flowPtr, "trypoping from all stack");
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
            /** debug*/
            logMF(flowBlockStack.top(), "pop lazy delete from stack");
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

        /**debug*/
        logMF(fb, "attach flowBlock to stack");

         /** push to stack in each case*/
        FLOW_BLOCK_TYPE flowType = fb->getFlowType();

        if (flowType == SEQUENTIAL ||
            flowType == PARALLEL_NO_SYN ||
            flowType == PARALLEL_AUTO_SYNC){
            logMF(fb, "attach flowBlock to PATTERN stack");
            patternFlowBlockStack.push(fb);
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

        logMF(fb, "detach flowBlock to stack");
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
        /**debug*/
        logMF(fb, "attach IF flowBlock to stack");


    }

    void Controller::on_attach_flowBlock_elif(FlowBlockElif *fb) {
        assert(fb != nullptr);
        assert(fb->getFlowType() == ELIF || fb->getFlowType() == ELSE);
        assert(!flowBlockStack.empty());
        /*** do not purify flow stack*/
        flowBlockStack.push(fb);
        logMF(fb, "attach ELIF flowBlock to stack");


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
        /**debug*/
        logMF(fb, "detach ELIF flowBlock to stack");
    }

    FLOW_BLOCK_TYPE Controller::get_top_pattern_flow_block_type(){
        if (patternFlowBlockStack.empty()){
            return DUMMY_BLOCK;
        }
        return patternFlowBlockStack.top()->getFlowType();
    }





}