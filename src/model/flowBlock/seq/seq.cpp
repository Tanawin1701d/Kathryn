//
// Created by tanawin on 5/12/2566.
//

#include "seq.h"
#include "model/controller/controller.h"

namespace kathryn{

    /**
     *
     *
     * sequence flow
     *
     *
     * */

    FlowBlockSeq::FlowBlockSeq(): FlowBlockBase(SEQUENTIAL,
                                                {
                                                        {FLOW_ST_BASE_STACK,
                                                         FLOW_ST_PATTERN_STACK},
                                                        FLOW_JO_SUB_FLOW,
                                                        true
                                                }),
                                  resultNodeWrap(nullptr) {}

    FlowBlockSeq::~FlowBlockSeq(){
        delete resultNodeWrap;
        for (auto seqEle : _subSeqMetas){
            delete seqEle;
        }
    }

    void FlowBlockSeq::add_basic_node(Node* node) {
        assert(node != nullptr);
        _subSeqMetas.push_back(new SequenceEleBasic(node));
        /** base function to notice existence of sub flow element*/
        FlowBlockBase::add_basic_node(node);
    }

    void FlowBlockSeq::add_sub_flow_block(FlowBlockBase *subBlock) {
        assert(subBlock != nullptr);
        _subSeqMetas.emplace_back(new SequenceEleFlowBlock(subBlock));
        /** base function to notice existence of sub flow block*/
        FlowBlockBase::add_sub_flow_block(subBlock);
    }

    NodeWrap* FlowBlockSeq::sumarize_block() {
        assert(resultNodeWrap != nullptr);
        return resultNodeWrap;
    }

    void FlowBlockSeq::on_attach_block() {
        _ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockSeq::on_detach_block() {
        _ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockSeq::build_hw_component() {
        mf_assert(!_subSeqMetas.empty(), "seqBlock has no assignment");
        assert(_con_blocks.empty());
        NodeWrapCycleDet cycleDet;
        /** generate hardware*/
        int idx = 0;
        for (auto& seqMeta: _subSeqMetas) {
            seqMeta->setIntReset(_int_nodes[INT_RESET]); //// set interrupt reset must be set before gennode
            seqMeta->setHoldNode(_hold_node);
            seqMeta->genNode(get_clock_mode());
            seqMeta->setIdentStateId(get_global_id(),idx++);
            seqMeta->addToCycleDet(cycleDet);
            seqMeta->addToSystemNodes(_sys_nodes);
        }
        /** generate forceExit Node*/
            /***check areThere forceExitNode*/
        std::vector<NodeWrap*> allNw;
        for(auto seqEle: _subSeqMetas){
            if (seqEle->isNodeWrap()){
                allNw.push_back(seqEle->getNodeWrap());
            }
        }
        gen_sum_force_exit_node(allNw);
        /** connect depend node chain*/
        for (size_t idx = 0; (idx+1) < _subSeqMetas.size(); idx++){
            auto lhsNodeWrapper = _subSeqMetas[idx];
            auto rhsNodeWrapper = _subSeqMetas[idx+1];
            rhsNodeWrapper->assignDependDent(lhsNodeWrapper);
        }
        if (is_there_intr_start()){
            _subSeqMetas[0]->assignIntStart(_int_nodes[INT_START]);
        }

        /** build new result NodeWrap*/
        resultNodeWrap = new NodeWrap();
        resultNodeWrap->add_entrace_nodes((*_subSeqMetas.begin())->getEntranceNodes());
        resultNodeWrap->add_exit_node((*_subSeqMetas.rbegin())->getStateFinishIden());
        resultNodeWrap->set_cycle_used(cycleDet.get_cycle_vertical());
        if (_are_there_force_exit)
            resultNodeWrap->add_force_exit_node(_force_exit_node);

    }



    std::string FlowBlockSeq::get_md_describe() {

        std::string ret;
        int eleCnt = 0;
        ret += "[ " + FlowBlockBase::get_md_ident_val() + " ]\n";
        for (auto seqEle: _subSeqMetas){
            ret += seqEle->getDescribe() + "\n";
            eleCnt++;
        }
        ret += get_md_describe_recur();
        return ret;

    }

    void FlowBlockSeq::add_md_log(MdLogVal *mdLogVal) {

        mdLogVal->addVal("[ " + FlowBlockBase::get_md_ident_val() + " ]");
        for (auto seqEle: _subSeqMetas){
            mdLogVal->addVal(seqEle->getDescribe());
        }
        if (resultNodeWrap->is_there_force_exit_node()){
            mdLogVal->addVal("forceExit is " + resultNodeWrap->get_force_exit_node()->get_md_ident_val() +
                                                   "  " +
                                                   resultNodeWrap->get_force_exit_node()->get_md_describe());
        }

        add_md_log_recur(mdLogVal);

    }

    void FlowBlockSeq::do_pre_function() {
        on_attach_block();
    }

    void FlowBlockSeq::do_post_function() {
        on_detach_block();
    }

}
