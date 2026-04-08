//
// Created by tanawin on 5/1/2567.
//

#include "cbreak.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockSCBreak::FlowBlockSCBreak():
    FlowBlockBase(EXITWHILE,
                  {
                          {FLOW_ST_BASE_STACK},
                          FLOW_JO_SUB_FLOW,
                          true
                  }),
    LoopStMacro() {}

    FlowBlockSCBreak::FlowBlockSCBreak(Operable& opr1):
            FlowBlockBase(EXITWHILE,
                          {
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_SUB_FLOW,
                                  true
                          }),
            LoopStMacro(),
            forceExitOpr(&opr1){}

    FlowBlockSCBreak::~FlowBlockSCBreak(){
        delete resultNodeWrap;
        delete breakNode;
        delete breakCondNode;
        delete normExitNode;
    }

    void FlowBlockSCBreak::add_basic_node(Node *node) {
        assert(false);
    }

    void FlowBlockSCBreak::add_sub_flow_block(FlowBlockBase *subBlock) {
        assert(false);
    }

    NodeWrap* FlowBlockSCBreak::sumarize_block() {
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockSCBreak::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockSCBreak::on_detach_block() {
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockSCBreak::build_hw_component() {

        assert(_con_blocks.empty());
        /**build internal node*/
        breakNode = new StateNode(get_clock_mode());
        add_sys_node(breakNode);
        fill_intr_reset_to_node_if_there(breakNode);

        normExitNode = new DummyNode(&makeOprVal("cbreakDum", 1, 0));
        add_sys_node(normExitNode);
        /////////normExitNode->addDependNode(breakNode);
        normExitNode->assign();

        /**build resultNodeWrap*/
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->add_entrace_node(breakNode);
        if (forceExitOpr == nullptr){
            resultNodeWrap->add_force_exit_node(breakNode);
        }else{
            breakCondNode = new PseudoNode(1, BITWISE_AND);
            breakCondNode->add_depend_node(breakNode, forceExitOpr);
            breakCondNode->assign();
            add_sys_node(breakCondNode);
            resultNodeWrap->add_force_exit_node(breakCondNode);
        }
        resultNodeWrap->add_exit_node(normExitNode);

    }

    std::string FlowBlockSCBreak::get_md_describe(){
        std::string ret;
        ret += "[breakNode is]"+ (breakNode != nullptr ? breakNode->get_md_describe() : "") + "\n";
        return ret;
    }

    void FlowBlockSCBreak::add_md_log(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        mdLogVal->addVal("breakNode is " +
                             (breakNode != nullptr ? breakNode->get_md_describe() : ""));
    }

    void FlowBlockSCBreak::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockSCBreak::do_post_function() {
        on_detach_block();
    }
}