//
// Created by tanawin on 6/12/2566.
//

#ifndef KATHRYN_IF_H
#define KATHRYN_IF_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#define cif(expr) for(auto kathrynBlock_cif = new FlowBlockIf(expr, CIF); kathrynBlock_cif->doPrePostFunction(); kathrynBlock_cif->step())
#define sif(expr) for(auto kathrynBlock_sif = new FlowBlockIf(expr, SIF); kathrynBlock_sif->doPrePostFunction(); kathrynBlock_sif->step())

namespace kathryn{

    class FlowBlockElif;

    class FlowBlockIf: public FlowBlockBase, public LoopStMacro{
    private:

        FlowBlockBase*          implicitFlowBlock = nullptr;
        std::vector<NodeWrap*>  allStatement; /// include current block and else block
        std::vector<Operable*>  allCondes; /// include condition of if block and elif block except else block
        std::vector<Operable*>  allPurifiedCondes;

        Node*       condNode       = nullptr;
        PseudoNode* exitNode       = nullptr;
        NodeWrap*   resultNodeWrap = nullptr;

    public:
        explicit FlowBlockIf(Operable& cond, FLOW_BLOCK_TYPE flowBlockType);
        ~FlowBlockIf() override;

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* fb) override;
        NodeWrap* sumarizeBlock() override;
        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** get describe*/
        std::string getMdDescribe() override;
        void addMdLog(MdLogVal *mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;
    };



}

#endif //KATHRYN_IF_H
