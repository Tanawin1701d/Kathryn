//
// Created by tanawin on 4/12/2566.
//

#ifndef KATHRYN_PAR_H
#define KATHRYN_PAR_H


#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"

namespace kathryn {

    class FlowBlockPar : public FlowBlockBase, public LoopStMacro{

        std::vector<Node*> _simpleAsm;

    public:

        explicit FlowBlockPar();

        /** override flow block base*/

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node);
        NodeWrapper* sumarizeBlock(); /// to interact from parrent block call

        /** on this block is start interact to controller*/
        void onAttachBlock();
        /** on leave this block*/
        void onDetachBlock();

        /** for module to build hardware component*/
        void buildHwComponent();

        /** Loop*/
        void doPreFunction();
        void doPostFunction();

    };


}

#endif //KATHRYN_PAR_H
