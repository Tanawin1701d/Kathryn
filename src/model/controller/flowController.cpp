//
// Created by tanawin on 3/12/2566.
//
#include "controller.h"
namespace kathryn{

    FlowBlockBase* Controller::getTopFlowBlock(){
        assert(!flowBlockStack.empty());
        return flowBlockStack.top();
    }

    void Controller::tryPopFromStack(FlowBlockBase* flowPtr,
                                     std::stack<FlowBlockBase*>& srcSt) {
        /**skip when empty skip when not match*/
        if (srcSt.empty() || (srcSt.top() != flowPtr))
            return;

        srcSt.pop();

    }

    void Controller::tryPopCtrlFlowFromAllStack(FlowBlockBase* flowPtr){
        tryPopFromStack(flowPtr, flowBlockStack);
        tryPopFromStack(flowPtr, patternFlowBlockStack);
        tryPopFromStack(flowPtr, condStlessFlowBlockStack);
    }



    void Controller::on_attach_flowBlock(FlowBlockBase* fb) {

        assert(fb != nullptr);

        if (!flowBlockStack.empty()){
            /** we must save head of flow block for deleting and debugging*/
            getTargetModuleEle().md->addFlowBlock(fb);
        }

         FLOW_BLOCK_TYPE flowType = fb->getFlowType();
         flowBlockStack.push(fb);

         /** push to stack in each case*/

         switch (flowType){

             case SEQUENTIAL :
             case PARALLEL   :
                 patternFlowBlockStack.push(fb);
                 break;
             case NO_STATE_IF :
                 condStlessFlowBlockStack.push(fb);
                 break;
         }

    }

    void Controller::on_detach_flowBlock(FlowBlockBase* fb) {
        assert(fb != nullptr);
        auto top= getTopFlowBlock();
        assert(top == fb);
        tryPopCtrlFlowFromAllStack(fb);

        if (!flowBlockStack.empty()){
            getTopFlowBlock()->addSubFlowBlock(fb);
        }
    }

    FLOW_BLOCK_TYPE Controller::get_top_pattern_flow_block_type(){
        if (patternFlowBlockStack.empty()){
            return DUMMY_BLOCK;
        }
        return getTopFlowBlock()->getFlowType();
    }



}