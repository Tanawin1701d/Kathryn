//
// Created by tanawin on 22/4/2567.
//

#include "doWhileBase.h"
#include "model/controller/controller.h"


namespace kathryn {

    FlowBlockDowhile::FlowBlockDowhile(Operable &condExpr,
                                       FLOW_BLOCK_TYPE fbt) :
            _condExpr(&condExpr),
            _purifiedCondExpr(purify_condition(&condExpr)),
            FlowBlockBase(fbt,
                          {
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_SUB_FLOW,
                                  true
                          }) {
        assert(_purifiedCondExpr != nullptr);
    }

    FlowBlockDowhile::~FlowBlockDowhile() {
        delete resultNodeWrapper;
        delete exitNode;
    }


    void FlowBlockDowhile::build_hw_component() {
        assert(_con_blocks.empty());
        assert(_sub_blocks.size() == 1);
        subBlockNodeWrap = _sub_blocks[0]->sumarize_block();
        assert(subBlockNodeWrap != nullptr);

        /***sub block depend lower deck is assume condition or*/
        subBlockNodeWrap->add_depend_node_to_all_node(subBlockNodeWrap->get_exit_node(),
                                                 subBlockNodeWrap->is_there_force_exit_node()
                                                 ? &((*_purifiedCondExpr) & (~(*subBlockNodeWrap->get_force_exit_node()->get_exit_opr_ptr())))
                                                 : _purifiedCondExpr);
        if (is_there_intr_start()) {
            subBlockNodeWrap->add_depend_node_to_all_node(_int_nodes[INT_START], nullptr);
        }
        /** exit node*/
        //////// no need reset signal
        exitNode = new PseudoNode(1, BITWISE_OR);
        exitNode->add_depend_node(subBlockNodeWrap->get_exit_node(), &(!(*_purifiedCondExpr)));
        if (subBlockNodeWrap->is_there_force_exit_node()){
            exitNode->add_depend_node(subBlockNodeWrap->get_force_exit_node(), nullptr);
        }
        exitNode->assign();
        add_sys_node(exitNode);
        ////////////////////////////////////////////////////////////////////
        resultNodeWrapper = new NodeWrap();
        resultNodeWrapper->transfer_ent_node_from(subBlockNodeWrap);
        resultNodeWrapper->add_exit_node(exitNode);

    }


    void FlowBlockDowhile::add_basic_node(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockDowhile::add_sub_flow_block(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::add_sub_flow_block(subBlock);
    }

    NodeWrap *FlowBlockDowhile::sumarize_block() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockDowhile::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->on_attach_block();
    }

    void FlowBlockDowhile::on_detach_block() {
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->on_detach_block();
        assert(isGetFlowBlockYet);
        _ctrl->on_detach_flowBlock(this);
    }


    void FlowBlockDowhile::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockDowhile::do_post_function() {
        on_detach_block();
    }

    void FlowBlockDowhile::add_md_log(MdLogVal *mdLogVal) {

        mdLogVal->addVal("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        mdLogVal->addVal("exitNode " + exitNode->get_md_ident_val() + " " + exitNode->get_md_describe());
        mdLogVal->addVal("resultNodeWrap is" +
                         resultNodeWrapper->get_md_ident_val() + " " + resultNodeWrapper->get_md_describe());

        auto subLog = mdLogVal->makeNewSubVal();
        implicitFlowBlock->add_md_log(subLog);

    }

}