//
// Created by tanawin on 12/1/2567.
//

#ifndef KATHRYN_ASMNODE_H
#define KATHRYN_ASMNODE_H

#include <utility>

#include "node.h"
#include "model/controller/asmMode.h"

namespace kathryn {


    struct AsmNode : Node {
        std::vector<AssignMeta*> _assign_metas; //// AssignMeta is must not use the same assign metas

        explicit AsmNode(AssignMeta *assign_meta) :
                Node(ASM_NODE),
                _assign_metas({assign_meta})
        {
            assert(assign_meta != nullptr);
            set_clock_mode(CM_CLK_UNUSED);

        }

        explicit AsmNode(std::vector<AssignMeta*> assign_metas):
                Node(ASM_NODE),
                _assign_metas(std::move(assign_metas)){

            for (auto* asmMeta: _assign_metas){
                assert(asmMeta != nullptr);

            }
            set_clock_mode(CM_CLK_UNUSED);

        }

        ~AsmNode()  = default;

        std::vector<AssignMeta*>& get_assign_metas_ref(){return _assign_metas;}

        void assign() override{assert(false);}

        /// when assign meta is extracted to join the bigger assign meta
        /// this Node will be useless
        void transfer_out_assign_meta_ownership(){_assign_metas.clear();}

        void add_specific_pre_condition(Operable* cond, int des_idx){
            assert(cond != nullptr);
            assert(des_idx >= 0 && des_idx < _assign_metas.size());
            _assign_metas[des_idx]->addSpecificPreCondition(cond);
        }

        /// TODO remove it because the assign operator handle it already
        void override_clock_mode(CLOCK_MODE mode){
            /** for asmNode clock_mode is first assign with it was built however; the flowblock should override the system
             * before the assignment is building*/
            // for (auto* assignMeta: _assign_metas){
                //assignMeta->clockMode = mode;
        // }
        }

        void assign_from_state_node(Operable* hold_signal, Operable* reset_signal){
            assert(_node_srcs.size() == 1);
            //assert(nodeSrcs[0].condition == nullptr);
            assert(_node_srcs[0].depend_node != nullptr);
            assert(!_assign_metas.empty());

            for (int assign_idx = 0; assign_idx < _assign_metas.size(); assign_idx++) {
                AssignMeta* assign_meta     = _assign_metas[assign_idx];
                /*** for reg <<= operator*/
                if (assign_meta->asmType == ASM_DIRECT){

                    /// bind node condition with pre_condition first\
                        /// handle condition and reset signal
                    Operable* cond_event_ptr = add_logic_with_output(_node_srcs[0].condition, nullptr, BITWISE_AND);
                    if (hold_signal != nullptr){
                        cond_event_ptr = add_logic_with_output(cond_event_ptr, &(~(*hold_signal)), BITWISE_AND);
                    }
                    if (reset_signal != nullptr){
                        cond_event_ptr = add_logic_with_output(cond_event_ptr, &(~(*reset_signal)), BITWISE_AND);
                    }
                        /// handle condition with state
                    cond_event_ptr = add_logic_with_output(cond_event_ptr, _node_srcs[0].depend_node->get_operating_state_ptr(), BITWISE_AND);
                    /// assign from current dependency
                    assert(assign_meta->preUpdateElement != nullptr);

                    /////// update new Condition
                    UpdateEventCond* update_with_state = new UpdateEventCond();
                    update_with_state->addSubStmt(cond_event_ptr, assign_meta->getCurrentEvent());
                    assign_meta->setNewEditingEvent(update_with_state);
                    assign_meta->finalUpdate();
                    // auto resultUpEvent = new UpdateEvent({
                    //     condEvent,
                    //     nodeSrcs[0].dependNode->getStateOperating(), /////// you can not simply use genExitOpr because it is used to colab with reset
                    //     &assignMeta->valueToAssign,
                    //     assignMeta->desSlice,
                    //     _asmPriority,
                    //     _asmId,
                    //     assignMeta->clockMode
                    // });
                    ////////////////////////////////////////////////////////////////////////////////////////
                /** for reg = operator*/
                }else if (assign_meta->asmType == ASM_EQ_DEPNODE){
                    //////////////// assign as same as node that have been assign
                    for (auto nodeSrc: _node_srcs[0].depend_node->_node_srcs) {

                        Operable* cond_event = add_logic_with_output(nodeSrc.condition, nullptr, BITWISE_AND);
                        if (hold_signal != nullptr){
                            cond_event = add_logic_with_output(cond_event, &(~(*hold_signal)), BITWISE_AND);
                        }
                        cond_event = add_logic_with_output(cond_event, nodeSrc.depend_node->get_exit_opr_ptr(), BITWISE_AND);

                        UpdateEventCond* update_with_state = new UpdateEventCond();
                        update_with_state->addSubStmt(cond_event, assign_meta->getCurrentEvent());
                        assign_meta->setNewEditingEvent(update_with_state);
                        assign_meta->finalUpdate();
                        // auto resultUpEvent = new UpdateEvent({
                        //      condEvent,
                        //      nodeSrc.dependNode->getExitOpr(), ///// it is supposed to sensitive to reset already?
                        //      &assignMeta->valueToAssign,
                        //      assignMeta->desSlice,
                        //      _asmPriority,
                        //      _asmId,
                        //      assignMeta->clockMode
                        // });
                        // assignMeta->updateEventsPool.push_back(resultUpEvent);
                    }
                }else{
                    assert(false);
                }
            }

            /*** no need to deal with rst event due to data self invoked*/
        }
        /** assign with no flow block related*/
        void dry_assign() override{
            assert(!_assign_metas.empty());
            assert(_node_srcs.empty());


            for (int assignIdx = 0; assignIdx < _assign_metas.size(); assignIdx++) {
                AssignMeta* assign_meta = _assign_metas[assignIdx];
                UpdateEventCond* update_with_pure_cond_state = new UpdateEventCond();
                update_with_pure_cond_state->addSubStmt(nullptr, assign_meta->getCurrentEvent());
                assign_meta->setNewEditingEvent(update_with_pure_cond_state);
                assign_meta->finalUpdate();
                //
                // auto resultUpEvent = new UpdateEvent({
                //                                              condEvent,
                //                                              nullptr,
                //                                              &assignMeta->valueToAssign,
                //                                              assignMeta->desSlice,
                //                                              _asmPriority,
                //     _asmId,
                // assignMeta->clockMode});
                //
                // assignMeta->updateEventsPool.push_back(resultUpEvent);
            }
        }

        int get_cycle_used() override { return 1; }

    };

}
#endif //KATHRYN_ASMNODE_H
