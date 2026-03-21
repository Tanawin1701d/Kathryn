//
// Created by tanawin on 10/1/26.
//

#include "ztate.h"
#include "model/controller/controller.h"


namespace kathryn{

FlowBlockZtate::FlowBlockZtate(Operable &ident_state):
            FlowBlockBase(ZTATE_BLOCK,{
                    {FLOW_ST_BASE_STACK},
                     FLOW_JO_EXT_FLOW,
                     true
             }),
            _identState(ident_state) {}

    FlowBlockZtate::~FlowBlockZtate() {}


    void FlowBlockZtate::add_element_in_flow_block(Node* node) {
        assert(false);
    }

    void FlowBlockZtate::add_sub_flow_block(FlowBlockBase *sub_block) {
        assert(sub_block != nullptr);
        assert(sub_block->get_flow_type() == ZCASE_BLOCK);
        /** call base function */
        FlowBlockBase::add_sub_flow_block(sub_block);

        auto* sub_block_zcase = static_cast<FlowBlockZCase*>(sub_block);

        std::vector<ClassAssignMeta*> class_ass_metas = sub_block_zcase->get_class_ass_metas();


        _caseMatchPool.push_back(sub_block_zcase->get_case_id());

        //// try to each assignment to group

        for(ClassAssignMeta* case_class_ass_meta: class_ass_metas){
            bool found = false;
            for(ZStateClassAsm* zstate_class_asm: _assignMetas){
                if (zstate_class_asm->is_joinable(case_class_ass_meta)){
                    zstate_class_asm->add_case_block(sub_block_zcase->get_case_id(), case_class_ass_meta);
                    found = true;
                    break;
                }
            }
            if(!found){
                auto* new_z_state_class_asm = new ZStateClassAsm(&_identState);
                _assignMetas.push_back(new_z_state_class_asm);
                new_z_state_class_asm->add_case_block(sub_block_zcase->get_case_id(), case_class_ass_meta);
            }
        }

    }

    void FlowBlockZtate::add_con_flow_block(FlowBlockBase *fb){
        assert(false);

    }

    void FlowBlockZtate::add_int_signal(INT_TYPE type, Operable* signal){
        mf_assert(!is_there_int_start(), "start interrupt can start in zblock");
        mf_assert(!is_there_int_rst(), "start interrupt can reset in zblock");
    }

    NodeWrap *FlowBlockZtate::sumarize_block() {
        assert(false);
    }

    void FlowBlockZtate::on_attach_block() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZtate::on_detach_block() {
        set_lazy_delete();
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZtate::build_hw_component() {
        assert(false);
    }

    std::string FlowBlockZtate::get_md_describe() {
        assert(false);
    }

    void FlowBlockZtate::add_md_log(MdLogVal *md_log_val) {
        assert(false);
    }

    void FlowBlockZtate::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockZtate::do_post_function() {
        on_detach_block();
    }

    std::vector<AsmNode*> FlowBlockZtate::extract(){

        ///// generate the node
        std::vector<AsmNode*> result;
        for (ZStateClassAsm* ztate_class_asm: _assignMetas){
            result.push_back(ztate_class_asm->create_asm_node(_caseMatchPool));
        }
        return result;

    }

}