//
// Created by tanawin on 6/12/2566.
//

#include "if.h"
#include "model/controller/controller.h"

namespace kathryn{


    FlowBlockIf::FlowBlockIf(Operable& cond,
                             FLOW_BLOCK_TYPE flowBlockType):
    FlowBlockBase(flowBlockType,
       {
               {FLOW_ST_BASE_STACK,
                             FLOW_ST_HEAD_COND_STACK},
               FLOW_JO_SUB_FLOW,
               true
       }){
        assert(flowBlockType == CIF || flowBlockType == SIF);
        allCondes.push_back(&cond);
        allPurifiedCondes.push_back(purify_condition(&cond));
    }

    FlowBlockIf::~FlowBlockIf(){
        delete condNode;
        delete exitNode;
        delete resultNodeWrap;
    }

    void FlowBlockIf::add_basic_node(Node *node) {
        assert(false); /** this flow type will generate single par or seq block*/
    }

    void FlowBlockIf::add_sub_flow_block(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        FlowBlockBase::add_sub_flow_block(subBlock);
    }

    void FlowBlockIf::add_con_flow_block(FlowBlockBase* conBlock){
        assert(conBlock != nullptr);
        assert(conBlock->get_flow_type() == CSELIF ||
               conBlock->get_flow_type() == CSELSE);
        /*** convert to elif block*/
        FlowBlockElif* elifBlock = (FlowBlockElif*)conBlock;
        /*** call base function*/
        FlowBlockBase::add_con_flow_block(elifBlock);
        /** push to if-else concern element*/
        /// allStatement.push_back(elifBlock->sumarizeBlock());
        if (elifBlock->get_condition() != nullptr) {
            allCondes.push_back(elifBlock->get_condition());
            allPurifiedCondes.push_back(
                    purify_condition(elifBlock->get_condition())
            );
        }

        assert(conBlock->get_sub_blocks_ref()[0] != nullptr);
    }

    NodeWrap* FlowBlockIf::sumarize_block() {
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockIf::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
        auto sb = gen_implicit_sub_blk(PARALLEL_NO_SYN);
        _implicit_flow_block = sb;
        sb->on_attach_block();
    }

    void FlowBlockIf::on_detach_block() {
        assert(_implicit_flow_block != nullptr);
        _implicit_flow_block->on_detach_block();
        ////// we will hold this end block will handle it
        set_lazy_delete();

        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockIf::build_hw_component() {
        /**summarize all block*/
        assert(_sub_blocks.size() == 1);
        _all_statement.push_back(_sub_blocks[0]->sumarize_block());
        for (auto conFlowBlock: _con_blocks){
            _all_statement.push_back(conFlowBlock->sumarize_block());
        }
        assert(!allCondes.empty());
        assert(allPurifiedCondes.size() == allCondes.size());
        assert(!_all_statement.empty());

        /***
         *
         * initialize node
         *
         * */

        if (get_flow_type() == CIF){
            condNode = new PseudoNode(1, BITWISE_OR);
            condNode->set_internal_ident("cifNode" + std::to_string(get_global_id()));
        }else if(get_flow_type() == SIF){
            condNode = new StateNode(get_clock_mode());
            condNode->set_internal_ident("sifNode" + std::to_string(get_global_id()));
            fill_intr_reset_to_node_if_there(condNode);
            fill_hold_to_node_if_there(condNode);
        }else{assert(false);}
        add_sys_node(condNode);

        if (is_there_intr_start()){
            condNode->add_depend_node(_int_nodes[INT_START], nullptr);
        }


        exitNode = new PseudoNode(1, BITWISE_OR);
        add_sys_node(exitNode);
        exitNode->set_internal_ident("ifExitNode" + std::to_string(get_global_id()));
        resultNodeWrap = new NodeWrap();


        /***
         *
         * subblock and Exit assign
         *
         * */

        /**add condition to state*/
        Operable* prevFalse = &(~(*allPurifiedCondes[0]));
        /** assign first first if*/
        _all_statement[0]->add_depend_node_to_all_node( condNode,allPurifiedCondes[0]);
        _all_statement[0]->assign_all_node();
        exitNode->add_depend_node(_all_statement[0]->get_exit_node(), nullptr);


        int statementId = 1;
        for (; statementId < _all_statement.size(); statementId++){
            if (statementId < allPurifiedCondes.size()) {
                _all_statement[statementId]->add_depend_node_to_all_node(
                        condNode,
                        &((*allPurifiedCondes[statementId]) & (*prevFalse)));
                _all_statement[statementId]->assign_all_node();
                exitNode->add_depend_node(_all_statement[statementId]->get_exit_node(), nullptr);
                prevFalse = &((*prevFalse) & ~(*allPurifiedCondes[statementId]));
            }else{
                /** case else statement*/
                assert(statementId == (allPurifiedCondes.size())); /// check no ambiguous statement
                _all_statement[statementId]->add_depend_node_to_all_node(
                        condNode,
                        prevFalse);
                _all_statement[statementId]->assign_all_node();
                exitNode->add_depend_node(_all_statement[statementId]->get_exit_node(), nullptr);
            }
        }



        if (_all_statement.size() == allPurifiedCondes.size()){
            /** there is no else node*/
            /** prev false is ready*/
            exitNode->add_depend_node(condNode, prevFalse);
        }
        exitNode->assign();

        /**
         *
         * result node wrap
         *
         *
         * */


        resultNodeWrap->add_entrace_node(condNode);
        resultNodeWrap->add_exit_node(exitNode);

        /**force exit condition*/
        gen_sum_force_exit_node(_all_statement);
        if (_are_there_force_exit)
            resultNodeWrap->add_force_exit_node(_force_exit_node);

        /**
         *
         *
         * cycle determiner
         *
         *
         * */
        NodeWrapCycleDet deter;
        deter.add_to_det(_all_statement);
        if(_all_statement.size() == allPurifiedCondes.size()){
            /** there is zero state node*/
            deter.add_to_det(0);
        }

        /**cycle determiner for node wrap*/
        int cycleUsed = deter.get_same_cycle_horizon();
        if (cycleUsed == IN_CONSIST_CYCLE_USED){
            resultNodeWrap->set_cycle_used(IN_CONSIST_CYCLE_USED);
        }else if (get_flow_type() == CIF){
            resultNodeWrap->set_cycle_used(cycleUsed);
        }else if (get_flow_type() == SIF){
            resultNodeWrap->set_cycle_used(cycleUsed+1);
        }else{
            assert(false);
        }

    }


    std::string FlowBlockIf::get_md_describe() {
        std::string ret;
        ret += "[ " + FlowBlockBase::get_md_ident_val() +" ]\n";
        ret += "exitNode is " + ((exitNode != nullptr) ? exitNode->get_md_ident_val()+ "  " + exitNode->get_md_describe(): "") + "\n";
        ret += "[implicitFlowBlock]" + _implicit_flow_block->get_md_describe() + "\n";
        return ret;
    }

    void FlowBlockIf::add_md_log(MdLogVal *mdLogVal) {
        assert(allCondes.size() == allPurifiedCondes.size());
        mdLogVal->addVal("[ " + FlowBlockBase::get_md_ident_val() +" ]");
        int cnt = 0;
        if (resultNodeWrap->is_there_force_exit_node()){
            mdLogVal->addVal("forceExit is " + resultNodeWrap->get_force_exit_node()->get_md_ident_val() +
                             "  " +
                             resultNodeWrap->get_force_exit_node()->get_md_describe());
        }
        for (auto sb : _sub_blocks){
            std::string subBlockHeaderDebug = "----> subblock " + std::to_string(cnt) + " condition ";
            if (cnt < allCondes.size()) {
                subBlockHeaderDebug += allCondes[cnt]->castToIdent()->getIdentDebugValue();
            }
            mdLogVal->addVal(subBlockHeaderDebug);

            sb->add_md_log(mdLogVal->makeNewSubVal());
            cnt++;
        }
    }

    void FlowBlockIf::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockIf::do_post_function() {
        on_detach_block();
    }

}