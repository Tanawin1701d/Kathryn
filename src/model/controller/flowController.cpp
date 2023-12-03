//
// Created by tanawin on 3/12/2566.
//
#include "controller.h"
namespace kathryn{

    void Controller::tryPopFromStack(const FlowBlockBasePtr &flowPtr,
                                     std::stack<FlowBlockBasePtr>&srcSt) {
        /**skip when empty skip when not match*/
        if (srcSt.empty() || (srcSt.top() != flowPtr))
            return;

        srcSt.pop();

    }

    void Controller::tryPopCtrlFlowFromAllStack(const FlowBlockBasePtr& flowPtr){
        tryPopFromStack(flowPtr, flowBlockStack);
        tryPopFromStack(flowPtr, patternFlowBlockStack);
        tryPopFromStack(flowPtr, condStlessFlowBlockStack);
    }



    void Controller::on_attach_flowBlock(const FlowBlockBasePtr& fb) {
        /**
         * todo integrity check
         * */
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

    void Controller::on_detach_flowBlock(const FlowBlockBasePtr& fb) {

        assert(!flowBlockStack.empty());
        auto top= flowBlockStack.top();
        assert(fb == top);

        tryPopCtrlFlowFromAllStack(fb);

        if (!flowBlockStack.empty()){
            flowBlockStack.top()->addElementInFlowBlock(top);
        }




    }



}