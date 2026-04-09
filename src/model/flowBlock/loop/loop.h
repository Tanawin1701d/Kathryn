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
    for(auto kathryn_block = new FlowBlockLoop(loopNumber); kathryn_block->do_pre_post_function(); kathryn_block->step()) \
        for (Operable& kathrynLoopName = kathryn_block->getLoopId(); !kathryn_block->isGottenLoopVar(); kathryn_block->setGetLoopVar())

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

        void build_hw_component() override;

        /** for controller add the local element to this sub block*/
        void add_basic_node(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* subBlock) override;
        NodeWrap* sumarize_block() override;

        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        ///////////void buildHwComponent() override;

        /** Loop macro to notice position of system*/
        void do_pre_function() override;
        void do_post_function() override;

        void add_md_log(MdLogVal* mdLogVal) override;

        Operable& getLoopId(){return *_loopId;}

        void setGetLoopVar(){ getLoopVar = true; }
        bool isGottenLoopVar(){return getLoopVar;}
        
    };

}

#endif //MODEL_FLOWBLOCK_LOOP_LOOP_H