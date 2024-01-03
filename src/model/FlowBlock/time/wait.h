//
// Created by tanawin on 2/1/2567.
//

#ifndef KATHRYN_WAIT_H
#define KATHRYN_WAIT_H

#include "model/FlowBlock/abstract/flowBlock_Base.h"


#define wait(cycle)

namespace kathryn{


    class FlowBlockCondWait : public FlowBlockBase{
    protected:
        /**result node wrap to sumarize this block*/
        NodeWrap* _resultNodeWrap = nullptr;
        /** exit condition*/
        Operable* _exitCond = nullptr;
        /** represent node*/
        WaitCondNode* _waitNode = nullptr;
    public:

        explicit FlowBlockCondWait(Operable* exitCond);

        NodeWrap* sumarizeBlock() override;
        void onAttachBlock() override;
        void onDetachBlock() override;
        void buildHwComponent() override;
    };

    class FlowBlockCycleWait : public FlowBlockBase{
    protected:
        /** result node wrap to summarize this block*/
        NodeWrap* _resultNodeWrap = nullptr;
        /**number cycle used*/
        int _cycleUsed = -1;
        /** wait cycle node*/
        WaitCycleNode* _waitNode = nullptr;

    public:

        explicit FlowBlockCycleWait(int cycleUsed);

        NodeWrap* sumarizeBlock() override;
        void onAttachBlock() override;
        void onDetachBlock() override;
        void buildHwComponent() override;

    };

}

#endif //KATHRYN_WAIT_H
