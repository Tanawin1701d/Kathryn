//
// Created by tanawin on 14/2/26.
//

#ifndef MODEL_FLOWBLOCK_LOOP_LOOP_H
#define MODEL_FLOWBLOCK_LOOP_LOOP_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/spReg/cntReg.h"

namespace kathryn{

    class FlowBlockLoop: public FlowBlockBase, public LoopStMacro{
    protected:
        int _loopCount = 0;
        //////// block
        FlowBlockBase* _implicitFlowBlock = nullptr;
        bool           _isGetFlowBlockYet = false;
        //////// nodes
        NodeWrap*      _resultNodeWrapper = nullptr;
        NodeWrap*      _subBlockNodeWrap  = nullptr;
        PseudoNode*    _entNode           = nullptr;
        PseudoNode*    _loopNode          = nullptr;
        CounterReg*    _cntNode           = nullptr;
        PseudoNode*    _exitNode          = nullptr;


        //// it is wrap is as same as result but it is used for loop assignment



    public:

        explicit FlowBlockLoop(int loopCount);
        ~FlowBlockLoop() override;

        void buildHwComponent() override;

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        NodeWrap* sumarizeBlock() override;

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        ///////////void buildHwComponent() override;

        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

        void addMdLog(MdLogVal* mdLogVal) override;
    };

}

#endif //MODEL_FLOWBLOCK_LOOP_LOOP_H