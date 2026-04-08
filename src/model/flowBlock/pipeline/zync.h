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

#define zync(zyncMeta)        for(auto kathrynBlock = new FlowBlockZyncBase(zyncMeta, nullptr); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define zyncc(zyncMeta, cond) for(auto kathrynBlock = new FlowBlockZyncBase(zyncMeta, &cond  ); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

//#define zync_con(cond)        for(auto kathrynBlock = new FlowBlockZyncBase(&cond);             kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    class FlowBlockZyncBase: public FlowBlockBase, public LoopStMacro{
    protected:
        const std::string _zyncName = "ZYNC_UNNAMED";
        SyncMeta& _syncMeta;
        Operable* _acceptCond = nullptr; ///// this condition must be true when the system is in prepSendNode,
                                            ///// if the preSend will not wait and not activate further layer
        ////// meta data
        bool autoActivatePipe = false;


        ////// node
        StateNode*  prepSendNode = nullptr;
        PseudoNode* exitNode     = nullptr;
        /////// expresion
        ////// node wrap for summarize
        NodeWrap*      resultNodeWrap    = nullptr;

    public:
        FlowBlockZyncBase(SyncMeta& syncMeta, Operable* acceptCond = nullptr);
        ~FlowBlockZyncBase() override;
        /** manage the system */
        void assignReadySignal();
        StateNode* getPreSendNode() { assert(prepSendNode != nullptr); return prepSendNode;}
        SyncMeta& getSyncMeta() const { return _syncMeta;}

        /** for controller add the local element to this sub block*/
        void add_sub_flow_block       (FlowBlockBase* subBlock) override;
        void add_con_flow_block       (FlowBlockBase* conBlock) override;
        NodeWrap* sumarize_block    () override;
        /** auto activate pipe*/
        void setAutoActivatePipe(){autoActivatePipe = true;}
        bool isAutoActivatePipe() const {return autoActivatePipe;}

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

#endif //SRC_MODEL_FLOW_BLOCK_PIPELINE_ZYNC_H