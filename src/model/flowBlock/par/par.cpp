//
// Created by tanawin on 4/12/2566.
//

#include "par.h"
#include "model/controller/controller.h"


namespace kathryn{


    FlowBlockPar::FlowBlockPar(FLOW_BLOCK_TYPE fbType):
    FlowBlockBase(fbType,
      {
              {FLOW_ST_BASE_STACK,
               FLOW_ST_PATTERN_STACK
              },
              FLOW_JO_SUB_FLOW,
              true
      })
      {
        assert((fbType == PARALLEL_AUTO_SYNC) ||
               (fbType == PARALLEL_NO_SYN));
    }

    FlowBlockPar::~FlowBlockPar(){
        delete resultNodeWrap;
        delete basicStNode;
        delete synNode;
        delete pseudoExitNode;
    }

    NodeWrap*
    FlowBlockPar::sumarize_block() {
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }


    void
    FlowBlockPar::on_attach_block(){
        _ctrl->on_attach_flowBlock(this);
    }

    void
    FlowBlockPar::on_detach_block() {
        _ctrl->on_detach_flowBlock(this);
    }

    void
    FlowBlockPar::build_hw_component() {
        mf_assert((!_basic_nodes.empty()) || (!_sub_blocks.empty()),
                 "parBlock has no assignment"
                 );
        assert(_con_blocks.empty());

        /**
         *
         * build node for basic assignment
         *
         * */
        if (!_basic_nodes.empty()){
            basicStNode = new StateNode(get_clock_mode());
            basicStNode->set_internal_ident("parStateReg_" + std::to_string(get_global_id()));
            add_sys_node(basicStNode);
            fill_intr_reset_to_node_if_there(basicStNode);
            fill_hold_to_node_if_there(basicStNode);
            /** add basic assignment to depend on stateNode*/
            for (auto nd : _basic_nodes){
                assert(nd->get_node_type() == ASM_NODE);
                basicStNode->add_slave_asm_node((AsmNode*)nd);
            }
        }


        /**
         *
         * gen node wrap
         *
         * */
        for (auto fb : _sub_blocks){
            NodeWrap* nw = fb->sumarize_block();
            assert(nw != nullptr);
            nodeWrapOfSubBlock.push_back(nw);
        }

        /**
         * gen force exit node
         * */
        gen_sum_force_exit_node(nodeWrapOfSubBlock);

        /**
         *
         * determine basic cycle used
         *
         * */
        NodeWrapCycleDet cycleDet;
        if (basicStNode != nullptr)
            cycleDet.addToDet(basicStNode);
        cycleDet.addToDet(nodeWrapOfSubBlock);
        cycleUsed = cycleDet.getMaxCycleHorizon();
        /**
         * build result node wrap entrance
         * */
        /*** entrance node management*/
        resultNodeWrap = new NodeWrap();
        if (basicStNode != nullptr) {
            resultNodeWrap->addEntraceNode(basicStNode);
            if (is_there_intr_start()){
                basicStNode->add_depend_node(_int_nodes[INT_START], nullptr);
            }
        }
        for (auto nw : nodeWrapOfSubBlock){
            resultNodeWrap->transferEntNodeFrom(nw);
            if (is_there_intr_start()){
                nw->addDependNodeToAllNode(_int_nodes[INT_START]);
            }
        }
        /** scan for master join block**/
        masterJoinFlowBlock = scan_master_join_sub_block();
        /**
         * assemble the result node wrap focused on synchronization and exit parameter
         * **/
        buildSyncNode();
        assignExitToRnw();
        assignCycleUsedToRnw();
        assignForceExitToRnw();
    }


    void FlowBlockPar::assignCycleUsedToRnw(){
        resultNodeWrap->setCycleUsed(cycleUsed);
    }

    void FlowBlockPar::assignForceExitToRnw() {
        if (_are_there_force_exit){
            resultNodeWrap->addForceExitNode(_force_exit_node);
        }
    }

    void
    FlowBlockPar::doPreFunction() {
        on_attach_block();
    }

    void
    FlowBlockPar::doPostFunction(){
        on_detach_block();
    }

    std::string FlowBlockPar::get_md_describe() {
        std::string ret;

        ret += "basicStateNode is " +
                ((basicStNode != nullptr) ?
                basicStNode->get_md_ident_val() + "  " + basicStNode->get_md_describe():
                ""
                ) +"\n";

        ret += "synNode is " +
                ((synNode != nullptr) ?
                synNode->get_md_ident_val() + "  " + synNode->get_md_describe():
                "") + "\n";

        ret += "pseudoExitNode is " +
                ((pseudoExitNode != nullptr) ?
                pseudoExitNode->get_md_ident_val() + "  " + pseudoExitNode->get_md_describe():
                "") + "\n";

        ret += get_md_describe_recur();
        ret += "\n";

        return ret;
    }

    void FlowBlockPar::add_md_log(MdLogVal *mdLogVal) {


        mdLogVal->addVal("[ " + FlowBlockBase::get_md_ident_val() + " ]");

        mdLogVal->addVal("basicStateNode is " +
                         ((basicStNode != nullptr) ?
                          basicStNode->get_md_ident_val() + "  " + basicStNode->get_md_describe():
                          ""
                         ));

        mdLogVal->addVal("synNode is " +
                         ((synNode != nullptr) ?
                          synNode->get_md_ident_val() + "  " + synNode->get_md_describe():
                          ""));

        mdLogVal->addVal("pseudoExitNode is " +
                         ((pseudoExitNode != nullptr) ?
                          pseudoExitNode->get_md_ident_val() + "  " + pseudoExitNode->get_md_describe():
                          ""));

        Node* exitNode = resultNodeWrap->getExitNode();
        mdLogVal->addVal("exit node is " +
                        ( (exitNode != nullptr) ?
                            exitNode->get_md_ident_val() + "  " + exitNode->get_md_describe():
                            ""));

        if (resultNodeWrap->isThereForceExitNode()){
            mdLogVal->addVal("forceExit is " + resultNodeWrap->getForceExitNode()->get_md_ident_val() +
                             "  " +
                             resultNodeWrap->getForceExitNode()->get_md_describe());
        }

        add_md_log_recur(mdLogVal);
    }

    /**
     *
     *
     * parallel block auto
     *
     *
     * */

    void FlowBlockParAuto::buildSyncNode() {
        int amt_block = ((basicStNode != nullptr) ? 1 : 0) +
                        (int)(nodeWrapOfSubBlock.size());
        /** build syn node if need*/
        if (
            (masterJoinFlowBlock == nullptr) && //// no user defined exit
            (cycleUsed == IN_CONSIST_CYCLE_USED) && ///// can't know the longest subblock
             (amt_block > 1) /** incase amt_block == 1 we don't have to sync*/
                ){
            /////// syn reg needed
            int synSize = amt_block;
            synNode = new SynNode(synSize, get_clock_mode());
            add_sys_node(synNode);
            fill_intr_reset_to_node_if_there(synNode);
            ///////[warning] this time we ensure that gensumforceExit is declared
            if(_force_exit_node){
                synNode->set_force_exit_event(_force_exit_node);
            }
            synNode->set_internal_ident(
                    "parSynNode_" +
                    std::to_string(get_global_id())
                    );
            /**syn node don't need to specify join operation due to it used own logic or*/
            if (basicStNode != nullptr){
                synNode->add_depend_node(basicStNode, nullptr);
            }
            for (auto nw : nodeWrapOfSubBlock){
                synNode->add_depend_node(nw->getExitNode(), nullptr);
            }
            ////// assign sync reg and sync node don't have to set join op because
            /////////// sync register will handle it
            synNode->assign();
        }

    }

    void FlowBlockParAuto::assignExitToRnw() {

        int amt_block = ((basicStNode != nullptr) ? 1 : 0) +
                        (int)(nodeWrapOfSubBlock.size());

        if (synNode != nullptr){
            resultNodeWrap->addExitNode(synNode);
        }else if (masterJoinFlowBlock != nullptr){ //// masterJoin is come from user declaration
            NodeWrap* joinnerNodeWrap = masterJoinFlowBlock->sumarize_block();
            Node* exitNode = joinnerNodeWrap->getExitNode();
            assert(exitNode != nullptr);
            resultNodeWrap->addExitNode(exitNode);
        }else{
            /** get Match allow nullptr*/
            Node* exitNode = nullptr;
            if (cycleUsed >= 0){    /////// can determine cycle
                exitNode = getMatchNodeFromNdsOrNws({basicStNode},
                                                    nodeWrapOfSubBlock,
                                                    cycleUsed);
            }else{ /////// cannot determine but have only one
                assert(amt_block == 1); //// in > 1
                exitNode = getAnyNodeFromNdsOrNws({basicStNode},
                                                  nodeWrapOfSubBlock);
            }
            assert(exitNode != nullptr);
            resultNodeWrap->addExitNode(exitNode);
        }

    }


    /**
     *
     *
     * parallel block no sync
     *
     *
     * */
    void FlowBlockParNoSync::assignExitToRnw() {

        int amt_block = ((basicStNode != nullptr) ? 1 : 0) +
                        (int)(nodeWrapOfSubBlock.size());

        /** get Match allow nullptr*/
        Node* exitNode = nullptr;
        if (cycleUsed >= 0){
            exitNode = getMatchNodeFromNdsOrNws({basicStNode},
                                                nodeWrapOfSubBlock,
                                                cycleUsed);
        }else if (amt_block == 1){
            assert(amt_block == 1);
            exitNode = getAnyNodeFromNdsOrNws({basicStNode},
                                              nodeWrapOfSubBlock);
        }else{
            assert(amt_block > 1);
            pseudoExitNode = new PseudoNode(1, BITWISE_OR);
            add_sys_node(pseudoExitNode);
            if (basicStNode != nullptr)
                pseudoExitNode->add_depend_node(basicStNode, nullptr);
            for (auto nw : nodeWrapOfSubBlock){
                pseudoExitNode->add_depend_node(nw->getExitNode(), nullptr);
            }
            pseudoExitNode->assign();
            exitNode  = pseudoExitNode;
        }
        assert(exitNode != nullptr);
        resultNodeWrap->addExitNode(exitNode);
    }
}
