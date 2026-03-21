//
// Created by tanawin on 2/12/2566.
//

#ifndef KATHRYN_SLICABLE_H
#define KATHRYN_SLICABLE_H

#include "model/hw_component/abstract/assignable.h"
#include "model/hw_component/abstract/operable.h"
#include "model/debugger/model_debugger.h"

namespace kathryn {

    template<typename T>
    class SliceAgent;

    template<typename T>
    class Slicable{

    private:
        std::vector<SliceAgent<T>*> agent_holders;/// it is used to hold slice_agent to prevent deletation
        const Slice _absSlice{}; /// it is absolute slice
    public:

        explicit Slicable(Slice abs_slice) : _absSlice(abs_slice){}
        virtual ~Slicable(){
            for (auto agent: agent_holders){
                delete agent;
            }
        }

        virtual SliceAgent<T>& operator() (int start, int stop) = 0;
        virtual SliceAgent<T>& operator() (int idx)             = 0;
        virtual SliceAgent<T>& operator() (Slice b)             = 0;

        /**this is used when Slicable of normal hwtype*/
        void add_agent_holder(SliceAgent<T>* agent){
            agent_holders.push_back(agent);
        }
        /** start and stop is */
        Slice get_abs_sub_slice(int start, int stop, Slice old_slice){
            return old_slice.get_sub_slice({start, stop});
        }
        Slice get_slice() const { return _absSlice; }
        ////void  set_slice(Slice slc) {_absSlice =  slc;}

    };

    /**key point is make the agent that transparent while routing
     * assignable for update_meta in assignable in agent is not used*/
    template<typename T>
    class SliceAgent : public AssignOpr<SliceAgent<T>>,public Assignable,
                       public Operable, public Slicable<T> {

    private:
        T* _master;
    public:
        SliceAgent(T* master, Slice slc) : _master(master), Slicable<T>(slc) {
            _master->Slicable<T>::add_agent_holder(this);
            AssignOpr<SliceAgent<T>>::set_master(this);
        };

        Operable* cast_to_operable(){
            return static_cast<Operable*>(this);
        }

        /** slicable overload*/

        LogicSimEngine*get_logic_sim_engine_from_opr_ptr() override{
            return _master->get_sim_engine_ptr();
        };

        SliceAgent<T>& operator() (int start, int stop) override{
            auto ret =  new SliceAgent<T>(_master,
                                          Slicable<T>::get_abs_sub_slice(start, stop, get_operable_slice())
                    );
            return *ret;
        }

        SliceAgent<T>& operator() (int idx) override{
            auto ret =  new SliceAgent<T>(_master,
                                          Slicable<T>::get_abs_sub_slice(idx, idx + 1, get_operable_slice())
                    );
            return *ret;
        }

        SliceAgent<T>& operator() (Slice sl) override{
            return operator() (sl.start, sl.stop);
        }

        /** override assignable*/
        void do_block_asm(Operable& src_opr, Slice des_slice) override {
            mf_assert(get_assign_mode() == AM_MOD, "agent can use operator <<= only in MD mode");
            assert(des_slice.get_size() <= Slicable<T>::get_slice().get_size());
            assert(des_slice.stop      <= Slicable<T>::get_slice().stop);
            _master->call_back_block_assign_from_agent(src_opr,des_slice);
        }

        void do_non_block_asm(Operable& src_opr, Slice des_slice) override{
            mf_assert(get_assign_mode() == AM_MOD, "agent can use operator <<= only in MD mode");
            assert(des_slice.get_size() <= Slicable<T>::get_slice().get_size());
            assert(des_slice.stop      <= Slicable<T>::get_slice().stop);
            _master->call_back_non_block_assign_from_agent(src_opr,des_slice);
        }

        void do_block_asm(Operable& src_opr,
                        std::vector<AssignMeta*>& result_meta_collector,
                        Slice  abs_src_slice,
                        Slice  abs_des_slice) override{
            _master->call_back_block_assign_from_agent(
                    src_opr,result_meta_collector,
                    abs_src_slice,abs_des_slice
            );
        }

        void do_non_block_asm(Operable& src_opr,
                           std::vector<AssignMeta*>& result_meta_collector,
                           Slice  abs_src_slice,
                           Slice  abs_des_slice) override{
            _master->call_back_non_block_assign_from_agent(
                    src_opr,result_meta_collector,
                    abs_src_slice,abs_des_slice
            );
        }

        CLOCK_MODE get_cur_assign_clk_mode() override {return _master->get_cur_assign_clk_mode();}

        SliceAgent<T>& operator = (Operable& b)     { AssignOpr<SliceAgent<T>>::operator_eq(b);   return *this;}
        SliceAgent<T>& operator = (ull b)           { AssignOpr<SliceAgent<T>>::operator_eq(b);      return *this;}
        SliceAgent<T>& operator = (SliceAgent<T>& b){ if (this == &b){return *this;} operator_eq(b); return *this;}

        [[nodiscard]] Slice get_assign_slice()  override { return  Slicable<T>::get_slice(); }

        /** override assign opr*/
        Assignable*get_assignable_from_assign_opr_ptr() override{
            return this;
        }

        /** operable override*/
        [[nodiscard]] Operable& get_exact_operable() const override { return *(Operable*)_master; }
        [[nodiscard]] Slice get_operable_slice() const override { return  Slicable<T>::get_slice(); }

        Operable* do_slice(Slice sl) override{
            auto& x = operator() (sl.start, sl.stop);
            return x.cast_to_operable();
        }

        Identifiable* cast_to_ident() override{
            return static_cast<Identifiable*>(_master);
        }

        Operable* check_short_circuit() override{
            return _master->check_short_circuit();
        }

        LogicGenBase* get_logic_gen_ptrBase() override{
            assert(false); ///// can't getlogic genbase from slice opr
        }
    };
}

#endif //KATHRYN_SLICABLE_H
