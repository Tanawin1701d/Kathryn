//
// Created by tanawin on 6/12/2566.
//

#ifndef KATHRYN_IF_H
#define KATHRYN_IF_H

#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"
#include "model/FlowBlock/abstract/node.h"

#define cif(expr) for(auto kathrynBlock = new FlowBlockIf(expr); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    class FlowBlockElif;

    class FlowBlockIf: public FlowBlockBase, public LoopStMacro{
    private:
        FlowBlockBase* implicitFlowBlock = nullptr;
        std::vector<NodeWrap*> allStatement; /// include current block and else block
        std::vector<Operable*>  allCondes; /// include condition of if block and elif block except else block

        NodeWrap* resultNodeWrapper = nullptr;

    public:
        explicit FlowBlockIf(Operable& cond);

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addElifNodeWrap(FlowBlockElif* fb);
        NodeWrap* sumarizeBlock() override;
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

#endif //KATHRYN_IF_H
