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
            purified_cur_cond(&(*purify_condition(&cond)))
    {
        ///assert(flow_block_type == ZIF);
        prev_falses.push_back(&(!(*purified_cur_cond)));
        assert(purified_cur_cond != nullptr);
    }

    FlowBlockZIF::~FlowBlockZIF() {}


    void FlowBlockZIF::add_element_in_flow_block(Node* node) {
        assert(node != nullptr);
        assert(node->get_node_type() == ASM_NODE);

        //////// cast meta data
        AsmNode* casted_node = (AsmNode*)node;
        FlowBlockBase::add_abandon_node(node);
        //////// try to add to
        try_add_or_create_asm_meta(casted_node, _assignMetas, purified_cur_cond);
    }

    void FlowBlockZIF::add_sub_flow_block(FlowBlockBase *sub_block) {
        assert(false);
    }

    void FlowBlockZIF::add_con_flow_block(FlowBlockBase *fb){
        assert(!last_zelif_detected);
        assert(!prev_falses.empty());
        assert(fb != nullptr);
        assert(fb->get_flow_type() == ZELIF || fb->get_flow_type() == ZELSE);
        /** call base function*/
        FlowBlockBase::add_con_flow_block(fb);
        FlowBlockZELIF* casted_zelif = (FlowBlockZELIF*)fb;

        Operable* prev_false = *prev_falses.rbegin();

        /**
         *  try to merge with exist node because we need to minimize the internal generation
         */

        std::vector<ZifClassAsm*> sub_class_asm_meta = casted_zelif->get_class_ass_metas();

        ///// it is not possible that slave will hit the same zif because the slave will handle it
        for (ZifClassAsm* slave_asm_meta: sub_class_asm_meta){
            bool found = false;
            for (ZifClassAsm* class_asm: _assignMetas){
                if (class_asm->is_joinable(slave_asm_meta)){
                    class_asm->add_zelif_stage(slave_asm_meta);
                    found = true;
                    break;
                }
            }
            if (!found){
                slave_asm_meta->condition = add_logic_with_output(prev_false, slave_asm_meta->condition, BITWISE_AND);
                _assignMetas.push_back(slave_asm_meta);
            }
        }
        /** generate next prev false*/
        if (casted_zelif->get_purified_cur_cond_ptr() == nullptr){
            last_zelif_detected = true;
        }else{
            prev_falses.push_back(
                    &((*prev_false) & (!(*casted_zelif->get_purified_cur_cond_ptr())))
            );
        }

    }

    void FlowBlockZIF::add_int_signal(INT_TYPE type, Operable* signal){
        mf_assert(!is_there_int_start(), "start interrupt can start in zblock");
        mf_assert(!is_there_int_rst(), "start interrupt can reset in zblock");
    }

    NodeWrap *FlowBlockZIF::sumarize_block() {
        assert(false);
    }

    void FlowBlockZIF::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZIF::on_detach_block() {
        set_lazy_delete();
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZIF::build_hw_component() {
        assert(false);
    }

    std::string FlowBlockZIF::get_md_describe() {
        assert(false);
    }

    void FlowBlockZIF::add_md_log(MdLogVal *md_log_val) {
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
        for (ZifClassAsm* zif_class_asm: _assignMetas){
            result.push_back(zif_class_asm->create_asm_node());
        }
        return result;

    }
}