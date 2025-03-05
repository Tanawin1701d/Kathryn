//
// Created by tanawin on 5/3/2025.
//

#ifndef SRC_MODEL_FLOWBLOCK_PIPELINE_PIPETRAN_H
#define SRC_MODEL_FLOWBLOCK_PIPELINE_PIPETRAN_H

////   SRC_MODEL_FLOWBLOCK_PIPELINE_PIPETRAN_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"


#define pipTran(pipeName)           for(auto kathrynBlock = new FlowBlockPipeTran (pipeName); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define pipTranWhen(pipeName, cond) for(auto kathrynBlock = new FlowBlockPipeTran (pipeName, cond); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    const char PIPE_TARGET_UNNAME[] = "PIPE_TARGET_UNNAMED";


    class FlowBlockPipeTran: public FlowBlockBase, public LoopStMacro{

    protected:
        const std::string _targetPipe = PIPE_TARGET_UNNAME;
        expression*       readySignal    = nullptr;
        Operable*         _customCond    = nullptr;
        //////////// node
        PseudoNode*       condNode       = nullptr;
        PseudoNode*       exitNode       = nullptr;
        StateNode*        basicStNode    = nullptr;
        NodeWrap*         resultNodeWrap = nullptr;

    public:
        explicit FlowBlockPipeTran(std::string  targetPipeName);
                 FlowBlockPipeTran(const std::string& targetPipeName, Operable& customCond);
        ~FlowBlockPipeTran() override;
        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* conBlock) override;
        void addAbandonFlowBlock(FlowBlockBase* abandonBlock) override;
        NodeWrap* sumarizeBlock() override;
        /**set activate bias usually used in init Pipe */
        Operable* createActivateCond() const;
        void buildReadySignal();
        Operable* getBlkReadySignal(){return readySignal;}
        std::string getTargetName()const{return _targetPipe;}
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




#endif //SRC_MODEL_FLOWBLOCK_PIPELINE_PIPETRAN_H
