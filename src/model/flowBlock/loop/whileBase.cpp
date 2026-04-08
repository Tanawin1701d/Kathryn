//
// Created by tanawin on 6/12/2566.
//

#include "whileBase.h"
#include "model/controller/controller.h"

namespace kathryn{

    FlowBlockWhile::FlowBlockWhile(Operable& condExpr,
                                   FLOW_BLOCK_TYPE fbt):
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

    FlowBlockWhile::FlowBlockWhile(bool fallTrue,
                                   FLOW_BLOCK_TYPE fbt):
            _fallTrue(fallTrue),
            FlowBlockBase(fbt,{
                                  {FLOW_ST_BASE_STACK},
                                  FLOW_JO_SUB_FLOW,
                                  true
                          }) {
        assert(fallTrue);
        assert(_purifiedCondExpr == nullptr);
    }



    FlowBlockWhile::~FlowBlockWhile() {
        delete resultNodeWrapper;
        delete conditionNode;
        delete exitDummy;
        delete exitNode;
    }


    void FlowBlockWhile::build_hw_component(){
        assert(_con_blocks.empty());
        assert(_sub_blocks.size() == 1);
        subBlockNodeWrap = _sub_blocks[0]->sumarize_block();
        assert(subBlockNodeWrap != nullptr);


        //** initialize node*/
        if (get_flow_type() == CWHILE){
            conditionNode = new PseudoNode(1, BITWISE_OR);
            conditionNode->set_internal_ident("cConNode" + std::to_string(get_global_id()));
        }else{////// SWHILE
            conditionNode = new StateNode(get_clock_mode());
            conditionNode->set_internal_ident("sConNode" + std::to_string(get_global_id()));
            fill_intr_reset_to_node_if_there(conditionNode);
            fill_hold_to_node_if_there(conditionNode);
        }
        add_sys_node(conditionNode);

        exitNode          = new PseudoNode(1, BITWISE_OR);
        add_sys_node(exitNode);
        resultNodeWrapper = new NodeWrap();
        ////////////////////////////////////////////////////////////////////

        /** do sub block dep init*/
        subBlockNodeWrap->add_depend_node_to_all_node(conditionNode, _purifiedCondExpr);
        subBlockNodeWrap->assign_all_node();

        /**do condition node Dep*/
            //// codition trigger from outside willbe trigger in upper node
        conditionNode->add_depend_node(subBlockNodeWrap->get_exit_node(),
                                     subBlockNodeWrap->is_there_force_exit_node()?
                                        ( &(~(*subBlockNodeWrap->get_force_exit_node()->get_exit_opr_ptr())) ):
                                        nullptr
                                     );
        if(is_there_intr_start()){
            conditionNode->add_depend_node(_int_nodes[INT_START], nullptr);
        }
        /**do exit NOde Dep*/
        if (!_fallTrue) {
            exitNode->add_depend_node(conditionNode, &(!(*_purifiedCondExpr)) );
        }
        if (subBlockNodeWrap->is_there_force_exit_node()){
            exitNode->add_depend_node(subBlockNodeWrap->get_force_exit_node(), nullptr);
        }

        if (_fallTrue && (!subBlockNodeWrap->is_there_force_exit_node())){
            ///////// incase there is no exit source we warning user that there is infinite loop
            /////////// TODO warning
            exitDummy = new DummyNode(&makeOprVal("exitDummy",1, 0));
            add_sys_node(exitDummy);
            exitNode->add_depend_node(exitDummy, nullptr);
        }

        exitNode->assign();


        resultNodeWrapper->add_entrace_node(conditionNode);
        resultNodeWrapper->add_exit_node(exitNode);

    }



    void FlowBlockWhile::add_basic_node(Node *node) {
        assert(false);
        /** cwhile not not except simple node due to implict added flowblock inside*/
    }

    void FlowBlockWhile::add_sub_flow_block(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        assert(!isGetFlowBlockYet);
        isGetFlowBlockYet = true;
        FlowBlockBase::add_sub_flow_block(subBlock);
    }

    NodeWrap* FlowBlockWhile::sumarize_block() {
        assert(resultNodeWrapper != nullptr);
        return resultNodeWrapper;
    }

    void FlowBlockWhile::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
        /** in cwhile we implcitcally add sub block to system*/
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        implicitFlowBlock = sb;
        sb->on_attach_block();
    }

    void FlowBlockWhile::on_detach_block() {
        assert(implicitFlowBlock != nullptr);
        implicitFlowBlock->on_detach_block();
        assert(isGetFlowBlockYet);
        _ctrl->on_detach_flowBlock(this);
    }


    void FlowBlockWhile::do_pre_function() {
        on_attach_block();
    }
    void FlowBlockWhile::do_post_function() {
        on_detach_block();
    }

    void FlowBlockWhile::add_md_log(MdLogVal* mdLogVal){

        mdLogVal->addVal("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        mdLogVal->addVal("conNode " + conditionNode->get_md_ident_val() + " " + conditionNode->get_md_describe());
        mdLogVal->addVal("exitNode " + exitNode->get_md_ident_val() + " " + exitNode->get_md_describe());
        mdLogVal->addVal("resultNodeWrap is" +
                         resultNodeWrapper->get_md_ident_val() + " " + resultNodeWrapper->get_md_describe());

        auto subLog = mdLogVal->makeNewSubVal();
        implicitFlowBlock->add_md_log(subLog);

    }

}