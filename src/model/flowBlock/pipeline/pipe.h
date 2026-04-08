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
#include "sim/modelSimEngine/flowBlock/pipeline/flowBlockPipProber.h"


#define pip(zyncMeta)       for(auto kathrynBlock = new FlowBlockPipeBase (zyncMeta); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define autoSync kathrynBlock->setAutoActivatePipe();


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
        void add_basic_node (Node* node) override;
        void add_sub_flow_block       (FlowBlockBase* subBlock) override;
        void add_con_flow_block       (FlowBlockBase* conBlock) override;
        void add_abandon_flow_block   (FlowBlockBase* abandonBlock) override;
        NodeWrap* sumarize_block    () override;
        /**set activate bias usually used in init Pipe */
        void      assignReadySignal ();

        std::string    getPipeName()          const{return _pipeName;}
        StateNode*     getWaitNode()          const{assert(waitNode != nullptr); return waitNode;}
        FlowBlockBase* getImplicitFlowBlock() const{return implicitFlowBlock;}
        /** auto activate pipe*/
        void      setAutoActivatePipe(){autoActivatePipe = true;}
        bool      isAutoActivatePipe() const{return autoActivatePipe;}
        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_master() override;
        void build_hw_component() override;
        /** get describe*/
        void add_md_log(MdLogVal* mdLogVal) override;
        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

    };

}


#endif //SRC_MODEL_FLOW_BLOCK_PIPELINE_PIPE_H
