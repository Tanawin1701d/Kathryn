//
// Created by tanawin on 10/1/26.
//

#ifndef MODEL_FLOWBLOCK_STATE_ZCASE_H
#define MODEL_FLOWBLOCK_STATE_ZCASE_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"

namespace kathryn{

    class FlowBlockZCase: public FlowBlockBase, public LoopStMacro{
    private:
        int _caseValue;
        std::vector<ClassAssignMeta*> _assignMetas;

    public:
        explicit FlowBlockZCase(int caseValue = -1);
        ~FlowBlockZCase() override;

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

        std::vector<ClassAssignMeta*> getClassAssMetas();
        int getCaseId() const;




    };

}

#endif //SRC_MODEL_FLOWBLOCK_STATE_ZCASE_H