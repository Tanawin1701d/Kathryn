//
// Created by tanawin on 1/3/2025.
//

#ifndef SRC_MODEL_FLOW_BLOCK_PIPELINE_PIPE_H
#define SRC_MODEL_FLOW_BLOCK_PIPELINE_PIPE_H


#include "syncMeta.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"


#define pip(zyncMeta)       for(auto kathrynBlock = new FlowBlockPipeBase (zyncMeta); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define autoStart kathrynBlock->setAutoActivatePipe();


namespace kathryn{


    const char PIPE_UNNAME[] = "PIPE_UNNAMED";


    class FlowBlockPipeBase: public FlowBlockBase,  public LoopStMacro{
    protected:
        SyncMeta& _syncMata;
        const std::string _pipeName = PIPE_UNNAME;
        ///////////// meta Data
        bool autoActivatePipe     = false;
        bool isGetFlowBlockYet    = false;
        ///////////// node
        PseudoNode* entNode       = nullptr; //// entrance node
        StateNode*  waitNode      = nullptr;
        DummyNode*  exitDummy     = nullptr; //// pipeline is perpeptual engine
        //////////// block implicit flow block
        FlowBlockBase* implicitFlowBlock = nullptr;
        NodeWrap*      subBlockNodeWrap  = nullptr;
        NodeWrap*      resultNodeWrap    = nullptr;

    public:
        explicit FlowBlockPipeBase(SyncMeta& syncMeta); ///// perpeptual loop indicate that it will loop when subblock is finish
        ~FlowBlockPipeBase() override;
        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock (Node* node) override;
        void addSubFlowBlock       (FlowBlockBase* subBlock) override;
        void addConFlowBlock       (FlowBlockBase* conBlock) override;
        void addAbandonFlowBlock   (FlowBlockBase* abandonBlock) override;
        NodeWrap* sumarizeBlock    () override;
        /**set activate bias usually used in init Pipe */
        void      assignReadySignal ();
        Operable* getBlkReadySignal() const{return _syncMata._syncMatched;}

        std::string getPipeName() const{return _pipeName;}
        /** auto activate pipe*/
        void      setAutoActivatePipe(){autoActivatePipe = true;}
        bool      isAutoActivatePipe() const{return autoActivatePipe;}
        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwMaster() override;
        void buildHwComponent() override;
        /** get describe*/
        void addMdLog(MdLogVal* mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

    };

}


#endif //SRC_MODEL_FLOW_BLOCK_PIPELINE_PIPE_H
