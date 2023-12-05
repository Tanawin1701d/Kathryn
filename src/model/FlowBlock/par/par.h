//
// Created by tanawin on 4/12/2566.
//

#ifndef KATHRYN_PAR_H
#define KATHRYN_PAR_H

#define par for(auto kathrynBlock = new FlowBlockPar(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"
#include "model/FlowBlock/abstract/stateReg.h"

namespace kathryn {

    class FlowBlockPar : public FlowBlockBase, public LoopStMacro{

        /** meta element*/
        std::vector<Node*> _simpleAsm;
        StateReg* stReg = nullptr;
        std::vector<StateReg*> synState;

        /** io element*/
        std::vector<UpdateEvent*> entranceEvent;
        expression* outputExpression;

    public:

        explicit FlowBlockPar();
        virtual ~FlowBlockPar(){
            for (auto nd: _simpleAsm){
                delete nd;
            }
        }

        /** override flow block base*/

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        NodeWrapper* sumarizeBlock() override; /// to interact from parrent block call

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;

        /** for module to build hardware component*/
        void buildHwComponent() override;

        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

    };

}



#endif //KATHRYN_PAR_H
