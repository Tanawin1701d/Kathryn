//
// Created by tanawin on 2/2/2567.
//

#include "zif.h"
#include "model/controller/controller.h"




namespace kathryn{


    FlowBlockZIF::FlowBlockZIF(Operable &cond):
            FlowBlockBase(ZIF,
                  {
                  { FLOW_ST_BASE_STACK,
                    FLOW_ST_HEAD_COND_STACK},
                  FLOW_JO_EXT_FLOW,
                  true
                  }),
            purifiedCurCond(&(*purify_condition(&cond)))
    {
        ///assert(flowBlockType == ZIF);
        prevFalses.push_back(&(!(*purifiedCurCond)));
        assert(purifiedCurCond != nullptr);
    }

    FlowBlockZIF::~FlowBlockZIF() {}


    void FlowBlockZIF::add_basic_node(Node* node) {
        assert(node != nullptr);
        assert(node->get_node_type() == ASM_NODE);

        //////// cast meta data
        AsmNode* castedNode = (AsmNode*)node;
        FlowBlockBase::add_abandon_node(node);
        //////// try to add to
        tryAddOrCreateAsmMeta(castedNode, _assignMetas, purifiedCurCond);
    }

    void FlowBlockZIF::add_sub_flow_block(FlowBlockBase *subBlock) {
        assert(false);
    }

    void FlowBlockZIF::add_con_flow_block(FlowBlockBase *fb){
        assert(!lastZelifDetected);
        assert(!prevFalses.empty());
        assert(fb != nullptr);
        assert(fb->get_flow_type() == ZELIF || fb->get_flow_type() == ZELSE);
        /** call base function*/
        FlowBlockBase::add_con_flow_block(fb);
        FlowBlockZELIF* castedZelif = (FlowBlockZELIF*)fb;

        Operable* prevFalse = *prevFalses.rbegin();

        /**
         *  try to merge with exist node because we need to minimize the internal generation
         */

        std::vector<ZifClassAsm*> subClassAsmMeta = castedZelif->getClassAssMetas();

        ///// it is not possible that slave will hit the same zif because the slave will handle it
        for (ZifClassAsm* slaveAsmMeta: subClassAsmMeta){
            bool found = false;
            for (ZifClassAsm* classAsm: _assignMetas){
                if (classAsm->isJoinable(slaveAsmMeta)){
                    classAsm->addZelifStage(slaveAsmMeta);
                    found = true;
                    break;
                }
            }
            if (!found){
                slaveAsmMeta->condition = add_logic_with_output(prevFalse, slaveAsmMeta->condition, BITWISE_AND);
                _assignMetas.push_back(slaveAsmMeta);
            }
        }
        /** generate next prev false*/
        if (castedZelif->getPurifiedCurCond() == nullptr){
            lastZelifDetected = true;
        }else{
            prevFalses.push_back(
                    &((*prevFalse) & (!(*castedZelif->getPurifiedCurCond())))
            );
        }

    }

    void FlowBlockZIF::add_intr_signal(INT_TYPE type, Operable* signal){
        mf_assert(!is_there_intr_start(), "start interrupt can start in zblock");
        mf_assert(!is_there_intr_rst(), "start interrupt can reset in zblock");
    }

    NodeWrap *FlowBlockZIF::sumarize_block() {
        assert(false);
    }

    void FlowBlockZIF::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZIF::on_detach_block() {
        set_lazy_delete();
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZIF::build_hw_component() {
        assert(false);
    }

    std::string FlowBlockZIF::get_md_describe() {
        assert(false);
    }

    void FlowBlockZIF::add_md_log(MdLogVal *mdLogVal) {
        assert(false);
    }

    void FlowBlockZIF::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockZIF::do_post_function() {
        on_detach_block();
    }

    std::vector<AsmNode*> FlowBlockZIF::extract(){

        ///// generate the node
        std::vector<AsmNode*> result;
        for (ZifClassAsm* zifClassAsm: _assignMetas){
            result.push_back(zifClassAsm->createAsmNode());
        }
        return result;

    }
}