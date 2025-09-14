//
// Created by tanawin on 13/9/25.
//

#ifndef SRC_MODEL_FLOW_BLOCK_PIPELINE_ZYNC_H
#define SRC_MODEL_FLOW_BLOCK_PIPELINE_ZYNC_H

#include "syncMeta.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"


namespace kathryn{

    class FlowBlockZyncBase: public FlowBlockBase, public LoopStMacro{
    protected:
        SyncMeta& _syncMeta;
        Operable* _aceeptCond    = nullptr; ///// this condition must be true when the system is in prepSendNode,
                                            ///// if the preSend will not wait and not activate further layer
        ////// node
        StateNode*  prepSendNode = nullptr;
        PseudoNode* exitNode     = nullptr;
        ////// node wrap for summarize
        NodeWrap*      resultNodeWrap    = nullptr;

    public:
        explicit FlowBlockZyncBase(SyncMeta& syncMeta);
        ~FlowBlockZyncBase() override;
        /** manage the system */
        void createReadySignal();
        void assignReadySignal();
        /** for controller add the local element to this sub block*/
        void addSubFlowBlock       (FlowBlockBase* subBlock) override;
        void addConFlowBlock       (FlowBlockBase* conBlock) override;
        NodeWrap* sumarizeBlock    () override;
        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** get describe*/
        void addMdLog(MdLogVal* mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;


    };

}

#endif //SRC_MODEL_FLOW_BLOCK_PIPELINE_ZYNC_H