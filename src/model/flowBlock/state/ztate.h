//
// Created by tanawin on 10/1/26.
//

#ifndef MODEL_FLOWBLOCK_STATE_ZTATE_H
#define MODEL_FLOWBLOCK_STATE_ZTATE_H

#include "ztateClassAsm.h"
#include "zcase.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"
#include "model/flowBlock/abstract/loopStMacro.h"

#define ztate(identState) for(auto kathrynBlock = new FlowBlockZtate(identState); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    class FlowBlockZtate: public FlowBlockBase,
                          public LoopStMacro{
    private:
        Operable& _identState;
        std::vector<ZStateClassAsm*> _assignMetas;

    public:
        explicit FlowBlockZtate(Operable& identState);
        ~FlowBlockZtate() override;

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* fb) override;
        void addIntSignal(INT_TYPE type, Operable* signal) override;
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
        /** extracted system*/
        std::vector<AsmNode*> extract() override;

    };



}

#endif //MODEL_FLOWBLOCK_STATE_ZTATE_H
