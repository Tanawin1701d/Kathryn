//
// Created by tanawin on 25/3/2567.
//

#ifndef KATHRYN_PIPEWRAPPER_H
#define KATHRYN_PIPEWRAPPER_H

#include "model/flowBlock/seq/seq.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#include "pipelineBlock.h"

#define pipWrap for(auto kathrynBlock_pipWrap = new FlowBlockPipeWrapper(); kathrynBlock_pipWrap->doPrePostFunction(); kathrynBlock_pipWrap->step())
#define nextPipReadySig getControllerPtr()->on_get_check_next_pipblk_ready_signal()

namespace kathryn{

    class FlowBlockPipeWrapper: public FlowBlockBase, public LoopStMacro{

        struct UserCheckNextSignal{
            int srcPipId = -1;
            int desPipId = -1;
            expression* expr = nullptr;

            explicit UserCheckNextSignal(int pipIdx);

            void connectSignal(std::vector<Pipe*>& allPip);

        };

    protected:

        std::vector<FlowBlockPipeBase*> _insidePipBlks;
        std::vector<NodeWrap*>          _nwOfPipBlks;
        std::vector<Pipe*>              _pipComs;
        std::vector<UserCheckNextSignal>_userCheckNextSignals;
        ///// collect meta data from next pipblock is ready to recv data

        /*
         *  |pip0|block0|pip1|block1|pip2|
         *
         * */
        Val*      _dummyExitVal   = nullptr;
        DummyNode* _dummyExitNode = nullptr;
        NodeWrap* _resultNodeWrap = nullptr;

    public:
        /** constructor*/
        explicit FlowBlockPipeWrapper();
        ~FlowBlockPipeWrapper() override;

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* conBlock) override;
        NodeWrap* sumarizeBlock() override;
        /** add next start signal*/
        expression& getNextPipBlockReadySignal();
        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/

        void buildHwMaster() override;
        void buildHwComponent() override;

        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

        /**get debug*/
        void addMdLog(MdLogVal* mdLogVal) override;

    };


}

#endif //KATHRYN_PIPEWRAPPER_H
