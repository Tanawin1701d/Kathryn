//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_ASSIGNABLE_H
#define KATHRYN_ASSIGNABLE_H

#include "vector"
#include "algorithm"

#include "operable.h"
#include "model/hw_component/abstract/slice.h"
#include "update_event.h"
#include "model/controller/clock_mode.h"
#include "ass_meta_mng.h"

namespace kathryn{





    /**
    * Assignable represent hardware component that can memorize logic value or
    *
    * */
    /** to make value when constant input is used*/
    Operable& get_match_assign_operable(ull value, int size);

    struct AsmNode;
    class Assignable{
    protected:
        UpdatePool _updatePool;
    public:

        explicit Assignable() = default;
        virtual ~Assignable(){
        }

        /** base function assign other operable to this operable*/
        virtual void do_block_asm   (Operable& src_opr, Slice des_slice) = 0;
        virtual void do_non_block_asm(Operable& src_opr, Slice des_slice) = 0;
        virtual void do_global_asm  (Operable& src_opr, Slice des_slice, ASM_TYPE asm_type){assert(false);} ///// typically, it is used in nest

        /** base function assign other operable to this operable
         * but do not communicate to controller, just give result assign meta to system*/
        virtual void do_block_asm   (Operable& src_opr,
                                   std::vector<AssignMeta*>& result_meta_collector, ////// result to assign assign meta
                                   Slice  abs_src_slice,
                                   Slice  abs_des_slice) = 0;

        virtual void do_non_block_asm(Operable& src_opr,
                                   std::vector<AssignMeta*>& result_meta_collector,
                                   Slice  abs_src_slice,
                                   Slice  abs_des_slice) = 0;

        /** global fucntion asign  other operable to this operable
         * but do not communicate to controller, just give result assign meta to system*/
        virtual void do_global_asm  (Operable& src_opr,
                                   std::vector<AssignMeta*>& result_meta_collector,
                                   Slice  abs_src_slice,
                                   Slice  abs_des_slice,
                                   ASM_TYPE asm_type);


        virtual void  assign_sim_value(ull        b)    {assert(false);}
        virtual void  assign_sim_value(ValRepBase value){assert(false);}

        virtual Slice get_assign_slice() = 0;

        /** update event management*/
        UpdatePool& get_update_meta(){ return _updatePool;}

        void add_update_meta(UpdateEventBase* event){_updatePool.add_update_event(event);}

        /** generate update metas*/
        virtual AssignMeta* generate_assign_meta(Operable& src_value, Slice des_slice,
                                               ASM_TYPE asm_type, CLOCK_MODE clock_mode){
            UpdateEventBasic* ueb = create_ue_helper(&src_value, des_slice, -1, clock_mode, true);
            return new AssignMeta(_updatePool, ueb, asm_type);
        }

        /** generate the atomic node that is used to represent  state in the system*/
        AsmNode* generate_basic_node(Operable& src_opr, Slice des_slice, ASM_TYPE asm_type);

        virtual CLOCK_MODE get_cur_assign_clk_mode() = 0;

        /***
         *
         * simulation task
         *
         * */

        void sort_up_event_by_priority(){
            _updatePool.sort_events();
        }

        // bool check_des_is_fully_assign_and_equal();


    };

    template<typename RET_TYPE>
    class AssignOpr{

    private:
        RET_TYPE* _master = nullptr;
    protected:
        void set_master(RET_TYPE* master){_master = master;}
        virtual Assignable*get_assignable_from_assign_opr_ptr() = 0;
    public:
        virtual RET_TYPE& operator <<= (Operable& b){
            Assignable* asb = get_assignable_from_assign_opr_ptr();
            Slice slc = asb->get_assign_slice();
            asb->do_block_asm(b, slc);
            assert(_master != nullptr);
            return *_master;
        }
        virtual RET_TYPE& operator <<= (ull b){
            Assignable* asb = get_assignable_from_assign_opr_ptr();
            Slice       slc = asb->get_assign_slice();
            Operable&   rhs = get_match_assign_operable(b, slc.get_size());
            asb->do_block_asm(rhs, slc);
            assert(_master != nullptr);
            return *_master;
        }
        virtual void operator_eq (Operable& b){
            Assignable* asb = get_assignable_from_assign_opr_ptr();
            Slice slc = asb->get_assign_slice();
            asb->do_non_block_asm(b, slc);
            assert(_master != nullptr);
        }
        virtual void operator_eq (ull b){
            Assignable *asb = get_assignable_from_assign_opr_ptr();
            if (get_assign_mode() == AM_MOD) {
                Slice slc = asb->get_assign_slice();
                Operable &rhs = get_match_assign_operable(b, slc.get_size());
                asb->do_non_block_asm(rhs, slc);
            }else if(get_assign_mode() == AM_SIM){
                asb->assign_sim_value(b);
            }
            assert(_master != nullptr);
        }


    };

    template<typename RET_TYPE>
    class AssignCallbackFromAgent{
    private:
        RET_TYPE* _master = nullptr;
    protected:
        void set_master(RET_TYPE* master){_master = master;}
        virtual Assignable*get_assignable_from_assign_callbacker_ptr() = 0;
    public:
        virtual void call_back_block_assign_from_agent(Operable& b, Slice abs_slice_of_host){
            get_assignable_from_assign_callbacker_ptr()->do_block_asm(b, abs_slice_of_host);
            assert(_master != nullptr);
        }
        virtual void call_back_non_block_assign_from_agent(Operable& b, Slice abs_slice_of_host){
            get_assignable_from_assign_callbacker_ptr()->do_non_block_asm(b, abs_slice_of_host);
            assert(_master != nullptr);
        }
        virtual void      call_back_block_assign_from_agent(Operable& src_opr,
                                                       std::vector<AssignMeta*>& result_meta_collector,
                                                       Slice  abs_src_slice,
                                                       Slice  abs_des_slice){
            get_assignable_from_assign_callbacker_ptr()->do_block_asm(src_opr, result_meta_collector, abs_src_slice, abs_des_slice);
            assert(_master != nullptr);
        }
        virtual void      call_back_non_block_assign_from_agent(Operable& src_opr,
                                                          std::vector<AssignMeta*>& result_meta_collector,
                                                          Slice  abs_src_slice,
                                                          Slice  abs_des_slice){
            get_assignable_from_assign_callbacker_ptr()->do_non_block_asm(src_opr, result_meta_collector, abs_src_slice, abs_des_slice);
            assert(_master != nullptr);
        }
    };




}

#endif //KATHRYN_ASSIGNABLE_H
