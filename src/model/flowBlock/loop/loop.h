//
// Created by tanawin on 14/2/26.
//

#ifndef MODEL_FLOWBLOCK_LOOP_LOOP_H
#define MODEL_FLOWBLOCK_LOOP_LOOP_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/cntNode.h"


#define cloop(kathrynLoopName, loopNumber) \
    for(auto kathrynBlock = new FlowBlockLoop(loopNumber); kathrynBlock->doPrePostFunction(); kathrynBlock->step()) \
        for (Operable& kathrynLoopName = kathrynBlock->getLoopId(); !kathrynBlock->isGottenLoopVar(); kathrynBlock->setGetLoopVar())

namespace kathryn{

    class FlowBlockLoop: public FlowBlockBase, public LoopStMacro{
    protected:
        int _loopCount = 0;
        bool getLoopVar = false;
        //////// block
        FlowBlockBase* _implicitFlowBlock = nullptr;
        bool           _isGetFlowBlockYet = false;
        //////// nodes
        NodeWrap*      _resultNodeWrapper = nullptr;
        NodeWrap*      _subBlockNodeWrap  = nullptr;
        PseudoNode*    _entNode           = nullptr;
        PseudoNode*    _loopNode          = nullptr;
        CounterNode*    _cntNode          = nullptr;
        PseudoNode*    _exitNode          = nullptr;
        //////// user wire
        expression*    _loopId            = nullptr;


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

        void add_md_log(MdLogVal* mdLogVal) override;

        Operable& getLoopId(){return *_loopId;}

        void setGetLoopVar(){ getLoopVar = true; }
        bool isGottenLoopVar(){return getLoopVar;}
        
    };

}

#endif //MODEL_FLOWBLOCK_LOOP_LOOP_H