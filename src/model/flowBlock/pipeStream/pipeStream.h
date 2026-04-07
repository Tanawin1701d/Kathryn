//
// Created by tanawin on 7/2/26.
//

#ifndef MODEL_FLOWBLOCK_PIPESTREAM_PIPESTREAM_H
#define MODEL_FLOWBLOCK_PIPESTREAM_PIPESTREAM_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "streamEle.h"

#define pipStream for(auto kathrynBlock = new FlowBlockPipeStream(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    class FlowBlockPipeStream : public FlowBlockBase, public LoopStMacro{
    private:
        std::vector<StreamEle*> _subStreamEles;
        NodeWrap*               _resultNodeWrap = nullptr;
        DummyNode*              _dummyNode      = nullptr;
    public:
        FlowBlockPipeStream();
        ~FlowBlockPipeStream() override;
        /** for controller add the local element to this sub block*/
        void add_basic_node(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* subBlock) override;
        NodeWrap* sumarize_block() override;

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_component() override;
        /** get describe*/
        std::string get_md_describe() override;
        void add_md_log(MdLogVal* mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;
    };

}


#endif //MODEL_FLOWBLOCK_PIPESTREAM_PIPESTREAM_H