//
// Created by tanawin on 27/11/25.
//

#ifndef MODEL_HWCOMPONENT_ABSTACT_ASSMETA_H
#define MODEL_HWCOMPONENT_ABSTACT_ASSMETA_H

#include "update_event.h"
//#include "model/flow_block/abstract/nodes/asm_node.h"

namespace kathryn{

    /** This is used to describe what and where to update that send to controller and let flow block determine*/
    enum ASM_TYPE{
        ASM_DIRECT = 0,
        ASM_EQ_DEPNODE = 1
    };

    struct AssignMeta{
        static inline ull ASSIGN_CNT = 0;
        UpdatePool&           event_pool;
        UpdateEventBasic*     input_element      = nullptr; //// it can be null if assign_meta is complex condition
        UpdateEventBase*      pre_update_element  = nullptr; //// the event want to beb
        ASM_TYPE              asm_type;

        AssignMeta(UpdatePool& u,
                   UpdateEventBasic* v,
                   ASM_TYPE at):
                    event_pool(u),
                    input_element(v),
                    pre_update_element(v),
                    asm_type(at){
            pre_update_element->set_sub_priority(ASSIGN_CNT);
            ASSIGN_CNT++;
        }

        AssignMeta(UpdateEventBase* v,
                   UpdatePool& u,
                   ASM_TYPE at,
                   ull cur_assign_cnt):
                    event_pool(u),
                    input_element(nullptr),
                    pre_update_element(v),
                    asm_type(at){
            pre_update_element->set_sub_priority(cur_assign_cnt);
        }

        ///// it is used to tell that two assignment is joinable into the generated (cpp or verilog block)
        [[nodiscard]]
        bool is_joinable(const AssignMeta& rhs) const{
            assert(pre_update_element != nullptr);
            return ((&event_pool) == (&rhs.event_pool)) &&
                   (asm_type == rhs.asm_type) &&
                   (pre_update_element->is_joinable(*rhs.get_current_event_ptr()));
        }

        void final_update(){
            event_pool.add_update_event(pre_update_element);
        }

        UpdatePool*get_event_pool_ptr(){return &event_pool;}
        [[nodiscard]]
        UpdateEventBase*get_current_event_ptr() const{return pre_update_element;}
        ASM_TYPE         get_asm_type() const{return asm_type;}
        ull              get_cur_assign_cnt() const{assert(pre_update_element != nullptr); return pre_update_element->get_sub_priority();}

        void             set_new_editing_event(UpdateEventBase* event){
            assert(event != nullptr);
            pre_update_element = event;
        }

        void add_specific_pre_condition(Operable* cond){
            assert(cond != nullptr);
            assert(pre_update_element != nullptr);
            pre_update_element = create_ue_helper(cond, nullptr, pre_update_element);

        }

        AssignMeta* mux(AssignMeta* right, Operable* select_left){
            UpdateEventCond* new_event  = create_mux_ue_helper(pre_update_element, right->pre_update_element, select_left);
            AssignMeta* new_ass_meta = new AssignMeta(new_event, event_pool, asm_type, pre_update_element->get_sub_priority());
            return new_ass_meta;

        }

    };

    struct ClassAssignMeta{
        //// this is the group of assign meta, the order is supposed to be inorder from user input
        /// the assign meta belongs to assign_meta
        std::vector<AssignMeta*> assign_metas;

        ClassAssignMeta(){}

        ClassAssignMeta(AssignMeta* assign_meta){
            assign_metas.push_back(assign_meta);
        }

        //////// all assignment Metas is supposed to be deleted because this class eventually creates
        //////// new one.
        virtual ~ClassAssignMeta(){
            for (auto* assign_meta: assign_metas){
                delete assign_meta;
            }
        }

        bool is_empty(){return assign_metas.empty();}

        int get_size() const{return assign_metas.size();}

        AssignMeta*get_sample_assign_meta_ptr(){
            assert(!assign_metas.empty());
            return assign_metas[0];
        }

        bool is_joinable(AssignMeta* sample){
            return get_sample_assign_meta_ptr()->is_joinable(*sample);
        }

        bool is_joinable(ClassAssignMeta* sample){
            assert(sample != nullptr);
            AssignMeta* our_sample  = get_sample_assign_meta_ptr();
            AssignMeta* your_sample = sample->get_sample_assign_meta_ptr();

            return our_sample->is_joinable(*your_sample);
        }

        void add_assign_meta(AssignMeta* assign_meta){
            if (!is_empty()){
                assert(is_joinable(assign_meta));
            }
            assign_metas.push_back(assign_meta);
        }

        ////// this is the goal of the system
        UpdateEventGrp* create_event_grp(){
            auto* update_grp = new UpdateEventGrp();
            for (auto* assign_meta: assign_metas){
                update_grp->add_sub_stmt(assign_meta->pre_update_element);
            }
            return update_grp;
        }

    };

    struct AsmNode;
    void try_add_or_create_asm_meta(
        AsmNode* asm_node,
        std::vector<ClassAssignMeta*>& assign_metas
    );

    //// std::vector<ClassAssignMeta*> classify_ass(std::vector<AssignMeta*>& base_metas);

}


#endif //KATHRYN_ASS_META_MNG_H
