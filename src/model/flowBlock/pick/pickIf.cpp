//
// Created by tanawin on 7/8/2024.
//

#include "pickIf.h"

#include <model/controller/controller.h>

namespace kathryn{


    FlowBlockPickCond::FlowBlockPickCond(Operable& cond):
    FlowBlockBase(PICK_WHEN,
        {
               {FLOW_ST_BASE_STACK},
               FLOW_JO_SUB_FLOW,
               true
        }
    ),
    condition(&cond){}

    FlowBlockPickCond::~FlowBlockPickCond(){
        //// delete resultNodeWrapper; it is derive from inside block do not delete
    }

    void FlowBlockPickCond::add_basic_node(Node* node){
        assert(false);
    }

    void FlowBlockPickCond::add_sub_flow_block(FlowBlockBase* subBlock){
        assert(subBlock != nullptr);
        FlowBlockBase::add_sub_flow_block(subBlock);
    }

    void FlowBlockPickCond::add_con_flow_block(FlowBlockBase* conBlock){
        assert(false);
    }

    NodeWrap* FlowBlockPickCond::sumarize_block(){
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockPickCond::on_attach_block(){
        _ctrl->on_attach_flowBlock(this);
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->on_attach_block();
    }

    void FlowBlockPickCond::on_detach_block(){
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->on_detach_block();
        /////// no need to set lazy delete
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockPickCond::build_hw_component(){
        assert(_con_blocks.empty());
        assert(!_sub_blocks.empty());
        mf_assert(!is_there_intr_start(), "start interrupt can start in elif block but can start in if block or block inside elif block");
        ///mfAssert(!isThereIntRst(), "start interrupt can reset in elif block but can start in if block or block inside elif block");
        resultNodeWrapper = _sub_blocks[0]->sumarize_block();

    }

    void FlowBlockPickCond::doPreFunction(){
        on_attach_block();
    }

    void FlowBlockPickCond::doPostFunction(){
        on_detach_block();
    }


}
