//
// Created by tanawin on 1/3/2025.
//

#ifndef SRC_MODEL_FLOW_BLOCK_PIPELINE_PIPE_H
#define SRC_MODEL_FLOW_BLOCK_PIPELINE_PIPE_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"


namespace kathryn{


    const char PIPE_UNNAME[] = "PIPE_UNNAMED";


    class FlowBlockPipe: public FlowBlockBase, public LoopStMacro{
    protected:
        ///////////// meta Data
        const std::string _pipeName = PIPE_UNNAME;
        bool isGetFlowBlockYet = false;
        ///////////// activate Condition
        bool isAutoActivate    = false;
        Operable* pipeActivateCond = nullptr; /////// get from pipe pooler
        ///////////// node
        PseudoNode* entNode  = nullptr; //// entrance node
        StateNode*  waitNode = nullptr;
        PseudoNode* pipeStartNode = nullptr;
        DummyNode*  exitDummy     = nullptr; //// pipeline is perpeptual engine


        //////////// block implicit flow block
        FlowBlockBase* implicitFlowBlock = nullptr;
        NodeWrap*      subBlockNodeWrap  = nullptr;
        NodeWrap*      resultNodeWrap    = nullptr;

    public:
        explicit FlowBlockPipe(const std::string& pipName);
        ~FlowBlockPipe() override;
        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* conBlock) override;
        void addAbandonFlowBlock(FlowBlockBase* abandonBlock) override;
        NodeWrap* sumarizeBlock() override;
        /**set activate bias usually used in init Pipe */
        void createActivateCond();
        void setAutoActivatePipe()      {isAutoActivate = true;}
        bool isAutoActivatePipe ()const {return isAutoActivate;}
        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** get describe*/
        std::string getMdDescribe() override;
        void addMdLog(MdLogVal* mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

    };



}


#endif //SRC_MODEL_FLOW_BLOCK_PIPELINE_PIPE_H
