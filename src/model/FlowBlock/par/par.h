//
// Created by tanawin on 4/12/2566.
//

#ifndef KATHRYN_PAR_H
#define KATHRYN_PAR_H


#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"
#include "model/FlowBlock/abstract/stateReg.h"
#include "model/FlowBlock/abstract/node.h"

#define par for(auto kathrynBlock = new FLowBlockParAuto(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())


namespace kathryn {

    class FlowBlockPar : public FlowBlockBase, public LoopStMacro{
    protected:

        /** metas element*/
        NodeWrap* resultNodeWrap = nullptr;

        /** check that subblock or basic node contain control flow operation*/
        std::vector<NodeWrap*> nodeWrapOfSubBlock;

        StateNode* basicStNode = nullptr;
        SynNode*   synNode     = nullptr;

    public:

        explicit FlowBlockPar();
        /** override flow block base*/
        NodeWrap* sumarizeBlock() override; /// to interact from parrent block call

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;

        /** for module to build hardware component*/
        void buildHwComponent() override;

        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

    };

    /** this parallel block auto check system*/
    class FLowBlockParAuto: public FlowBlockPar{

    public:

        explicit FLowBlockParAuto(): FlowBlockPar(){}

        void buildHwComponent() override;

    };

}



#endif //KATHRYN_PAR_H
