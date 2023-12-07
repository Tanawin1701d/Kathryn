//
// Created by tanawin on 6/12/2566.
//

#ifndef KATHRYN_IFELSE_H
#define KATHRYN_IFELSE_H

#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"


namespace kathryn{

    class FlowBlockElif;
    class FlowBlockElse;


    class FlowBlockIf: public FlowBlockBase, public LoopStMacro{
    private:
        bool isGetFlowBlockYet = false;
        Operable* _cond;
        std::vector<NodeWrap*> allStatement; /// include current block and else block
        std::vector<Operable*>  allCondes
        NodeWrap* resultNodeWrapper = nullptr;

    public:
        explicit FlowBlockIf(expression& cond);

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addElifNodeWrap
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

        /**add elif element*/
        void addElifElement(FlowBlockElif* elifClass);
        /**ad else Element*/
        void addElseElement(FlowBlockElse* elseClass);

    };

}

#endif //KATHRYN_IFELSE_H
