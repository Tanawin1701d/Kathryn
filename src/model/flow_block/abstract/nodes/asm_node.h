//
// Created by tanawin on 12/1/2567.
//

#ifndef KATHRYN_ASMNODE_H
#define KATHRYN_ASMNODE_H

#include "utility"

#include "node.h"
#include "model/controller/asm_mode.h"

namespace kathryn {


    struct AsmNode : Node {
        std::vector<AssignMeta*> _assignMetas; //// AssignMeta is must not use the same assign metas

        ////// it will used as order of the assignment node
        ////// when there are multiple assigns in the system
        //// TODO add per element metadata

        explicit AsmNode(AssignMeta *assign_meta) :
                Node(ASM_NODE),
                _assignMetas({assign_meta})
        {
            assert(assign_meta != nullptr);
            set_clock_mode(CM_CLK_UNUSED);

        }

        explicit AsmNode(std::vector<AssignMeta*> assign_metas):
                Node(ASM_NODE),
                _assignMetas(std::move(assign_metas)){

            for (auto* asm_meta: _assignMetas){
                assert(asm_meta != nullptr);

            }
            set_clock_mode(CM_CLK_UNUSED);

        }

        ~AsmNode()  = default;

        std::vector<AssignMeta*>& get_assign_metas(){return _assignMetas;}

        void assign() override{
            assert(false);
        }

        void transfer_out_assign_meta_ownership(){_assignMetas.clear();}

        void add_specific_pre_condition(Operable* cond, int des_idx){

            assert(cond != nullptr);
            assert(des_idx >= 0 && des_idx < _assignMetas.size());
            _assignMetas[des_idx]->add_specific_pre_condition(cond);

        }

        void override_clock_mode(CLOCK_MODE mode){
            /** for asm_node it is first assign with it was built however; the flowblock should override the system
             * before the assignment is building*/
            for (auto* assign_meta: _assignMetas){
                //assign_meta->clock_mode = mode;
            }
        }

        void assign_from_state_node(Operable* hold_signal, Operable* reset_signal){
            assert(node_srcs.size() == 1);
            //assert(node_srcs[0].condition == nullptr);
            assert(node_srcs[0].depend_node != nullptr);
            assert(!_assignMetas.empty());

            for (int assign_idx = 0; assign_idx < _assignMetas.size(); assign_idx++) {
                AssignMeta* assign_meta     = _assignMetas[assign_idx];
                /*** for reg <<= operator*/
                if (assign_meta->asm_type == ASM_DIRECT){

                    ////// bind node condition with pre_condition first\
                    ////////// handle condition and reset signal
                    Operable* cond_event = add_logic_with_output(node_srcs[0].condition, nullptr, BITWISE_AND);
                    if (hold_signal != nullptr){
                        cond_event = add_logic_with_output(cond_event, &(~(*hold_signal)), BITWISE_AND);
                    }
                    if (reset_signal != nullptr){
                        cond_event = add_logic_with_output(cond_event, &(~(*reset_signal)), BITWISE_AND);
                    }
                    ////////// handle condition with state
                    cond_event = add_logic_with_output(cond_event, node_srcs[0].depend_node->get_state_operating_ptr(), BITWISE_AND);
                    ///////////// assign from current dependency
                    assert(assign_meta->pre_update_element != nullptr);

                    /////// update new Condition
                    UpdateEventCond* update_with_state = new UpdateEventCond();
                    update_with_state->add_sub_stmt(cond_event, assign_meta->get_current_event_ptr());
                    assign_meta->set_new_editing_event(update_with_state);
                    assign_meta->final_update();
                    // auto result_up_event = new UpdateEvent({
                    //     cond_event,
                    //     node_srcs[0].depend_node->get_state_operating_ptr(), /////// you can not simply use gen_exit_opr because it is used to colab with reset
                    //     &assign_meta->value_to_assign,
                    //     assign_meta->des_slice,
                    //     _asmPriority,
                    //     _asmId,
                    //     assign_meta->clock_mode
                    // });
                    ////////////////////////////////////////////////////////////////////////////////////////
                /** for reg = operator*/
                }else if (assign_meta->asm_type == ASM_EQ_DEPNODE){
                    //////////////// assign as same as node that have been assign
                    for (auto node_src: node_srcs[0].depend_node->node_srcs) {

                        Operable* cond_event = add_logic_with_output(node_src.condition, nullptr, BITWISE_AND);
                        if (hold_signal != nullptr){
                            cond_event = add_logic_with_output(cond_event, &(~(*hold_signal)), BITWISE_AND);
                        }
                        cond_event = add_logic_with_output(cond_event, node_src.depend_node->get_exit_opr_ptr(), BITWISE_AND);

                        UpdateEventCond* update_with_state = new UpdateEventCond();
                        update_with_state->add_sub_stmt(cond_event, assign_meta->get_current_event_ptr());
                        assign_meta->set_new_editing_event(update_with_state);
                        assign_meta->final_update();
                        // auto result_up_event = new UpdateEvent({
                        //      cond_event,
                        //      node_src.depend_node->get_exit_opr_ptr(), ///// it is supposed to sensitive to reset already?
                        //      &assign_meta->value_to_assign,
                        //      assign_meta->des_slice,
                        //      _asmPriority,
                        //      _asmId,
                        //      assign_meta->clock_mode
                        // });
                        // assign_meta->update_events_pool.push_back(result_up_event);
                    }
                }else{
                    assert(false);
                }
            }

            /*** no need to deal with rst event due to data self invoked*/
        }
        /** assign with no flow block related*/
        void dry_assign() override{
            assert(!_assignMetas.empty());
            assert(node_srcs.empty());


            for (int assign_idx = 0; assign_idx < _assignMetas.size(); assign_idx++) {
                AssignMeta* assign_meta = _assignMetas[assign_idx];
                UpdateEventCond* update_with_pure_cond_state = new UpdateEventCond();
                update_with_pure_cond_state->add_sub_stmt(nullptr, assign_meta->get_current_event_ptr());
                assign_meta->set_new_editing_event(update_with_pure_cond_state);
                assign_meta->final_update();
                //
                // auto result_up_event = new UpdateEvent({
                //                                              cond_event,
                //                                              nullptr,
                //                                              &assign_meta->value_to_assign,
                //                                              assign_meta->des_slice,
                //                                              _asmPriority,
                //     _asmId,
                // assign_meta->clock_mode});
                //
                // assign_meta->update_events_pool.push_back(result_up_event);
            }
        }

        int get_cycle_used() override { return 1; }

    };

}
#endif //KATHRYN_ASMNODE_H
