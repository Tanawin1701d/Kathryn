//
// Created by tanawin on 5/12/2566.
//

#ifndef MODEL_FLOWBLOCK_SEQ_SEQ_H
#define MODEL_FLOWBLOCK_SEQ_SEQ_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"
#include "seqEle.h"


#define seq for(auto kathrynBlock = new FlowBlockSeq(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn {



    class FlowBlockSeq : public FlowBlockBase, public LoopStMacro{
    private:

        std::vector<SequenceEle*> _subSeqMetas;
        NodeWrap*                resultNodeWrap = nullptr;


    public:
        explicit FlowBlockSeq();
        ~FlowBlockSeq() override;
        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        NodeWrap* sumarizeBlock() override;

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** get describe*/
        std::string get_md_describe() override;
        void add_md_log(MdLogVal* mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;
    };


}

#endif //MODEL_FLOWBLOCK_SEQ_SEQ_H
