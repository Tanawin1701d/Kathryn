//
// Created by tanawin on 10/1/26.
//

#include "ztate.h"
#include "model/controller/controller.h"


namespace kathryn{

FlowBlockZtate::FlowBlockZtate(Operable &identState):
            FlowBlockBase(ZTATE_BLOCK,{
                    {FLOW_ST_BASE_STACK},
                     FLOW_JO_EXT_FLOW,
                     true
             }),
            _identState(identState) {}

    FlowBlockZtate::~FlowBlockZtate() {}


    void FlowBlockZtate::add_basic_node(Node* node) {
        assert(false);
    }

    void FlowBlockZtate::add_sub_flow_block(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(subBlock->get_flow_type() == ZCASE_BLOCK);
        /** call base function */
        FlowBlockBase::add_sub_flow_block(subBlock);

        auto* subBlockZcase = static_cast<FlowBlockZCase*>(subBlock);

        std::vector<ClassAssignMeta*> classAssMetas = subBlockZcase->getClassAssMetas();


        _caseMatchPool.push_back(subBlockZcase->getCaseId());

        //// try to each assignment to group

        for(ClassAssignMeta* caseClassAssMeta: classAssMetas){
            bool found = false;
            for(ZStateClassAsm* zstateClassAsm: _assignMetas){
                if (zstateClassAsm->isJoinable(caseClassAssMeta)){
                    zstateClassAsm->addCaseBlock(subBlockZcase->getCaseId(), caseClassAssMeta);
                    found = true;
                    break;
                }
            }
            if(!found){
                auto* newZStateClassAsm = new ZStateClassAsm(&_identState);
                _assignMetas.push_back(newZStateClassAsm);
                newZStateClassAsm->addCaseBlock(subBlockZcase->getCaseId(), caseClassAssMeta);
            }
        }

    }

    void FlowBlockZtate::add_con_flow_block(FlowBlockBase *fb){
        assert(false);

    }

    void FlowBlockZtate::add_intr_signal(INT_TYPE type, Operable* signal){
        mf_assert(!is_there_intr_start(), "start interrupt can start in zblock");
        mf_assert(!is_there_intr_rst(), "start interrupt can reset in zblock");
    }

    NodeWrap *FlowBlockZtate::sumarize_block() {
        assert(false);
    }

    void FlowBlockZtate::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockZtate::on_detach_block() {
        set_lazy_delete();
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockZtate::build_hw_component() {
        assert(false);
    }

    std::string FlowBlockZtate::get_md_describe() {
        assert(false);
    }

    void FlowBlockZtate::add_md_log(MdLogVal *mdLogVal) {
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
        for (ZStateClassAsm* ztateClassAsm: _assignMetas){
            result.push_back(ztateClassAsm->createAsmNode(_caseMatchPool));
        }
        return result;

    }

}