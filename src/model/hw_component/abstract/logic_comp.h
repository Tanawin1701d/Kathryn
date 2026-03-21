//
// Created by tanawin on 10/12/2566.
//

#ifndef KATHRYN_LOGICCOMP_H
#define KATHRYN_LOGICCOMP_H

#include "model/hw_component/abstract/model_mode.h"
#include "model/hw_component/abstract/slicable.h"
#include "model/hw_component/abstract/operation.h"
#include "model/hw_component/abstract/assignable.h"
#include "model/hw_component/abstract/operable.h"
#include "model/hw_component/abstract/identifiable.h"
#include "model/controller/con_interf/controller_itf.h"
#include "model/debugger/model_debugger.h"
#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"
#include "util/numberic/num_convert.h"

#include "gen/proxy_hw_comp/abstract/logic_gen_base.h"
// #include "gen/proxy_hw_comp/expression/expr_gen.h"
// #include "gen/proxy_hw_comp/expression/nest_gen.h"
// #include "gen/proxy_hw_comp/register/reg_gen.h"
// #include "gen/proxy_hw_comp/value/value_gen.h"
// #include "gen/proxy_hw_comp/wire/wire_gen.h"
// #include "gen/proxy_hw_comp/mem_block/mem_gen.h"
// #include "gen/proxy_hw_comp/mem_block/mem_agent_gen.h"


namespace kathryn{

    template<typename TYPE_COMP>
    class LogicComp : public AssignOpr<TYPE_COMP>,
                      public Assignable,
                      public Operable,
                      public Slicable<TYPE_COMP>,
                      public AssignCallbackFromAgent<TYPE_COMP>,
                      public Identifiable,
                      public HwCompControllerItf,
                      public ModelDebuggable,
                      public LogicSimEngineInterface,
                      public LogicGenInterface{
    protected:
        LogicSimEngine* _simEngine =  nullptr;
        LogicGenBase*   _genEngine =  nullptr;

    public:

        explicit LogicComp(Slice slc,
                           HW_COMPONENT_TYPE hw_type,
                           LogicSimEngine* sim_engine,
                           bool required_alloc_check):
                AssignOpr<TYPE_COMP>(),
                Assignable(),
                Operable(),
                Slicable<TYPE_COMP>(slc),
                Identifiable(hw_type),
                HwCompControllerItf(required_alloc_check),
                ////LogicSimEngine(slc.get_size(), sig_type, sim_for_next),
                ModelDebuggable(),
                _simEngine(sim_engine)
                            {}

        virtual ~LogicComp() {
            delete _simEngine;
            delete _genEngine;
        }


        /** simulation engine override*/

        LogicSimEngine* get_sim_engine_ptr() override{
            return _simEngine;
        }

        void mark_sv(const std::string& key) override{
            _simEngine->mark_sv(key);
        }


        ///////////////////////////////////////////

        LogicGenBase* get_logic_gen_ptr() override{
            return _genEngine;
        }

        LogicGenBase* get_logic_gen_ptrBase() override{
            return _genEngine;
        }


        /** iterable override*/

        std::string get_md_ident_val() override{
            return get_ident_debug_value();
        }

        Operable* cast_to_opr(){
            return static_cast<Operable*>(this);
        }

        LogicSimEngine*get_logic_sim_engine_from_opr_ptr() override{
            return _simEngine;
        }

        /** operable override*/
        Slice get_operable_slice() const override{
            return Slicable<TYPE_COMP>::get_slice();
        }

        Operable& get_exact_operable() const override{
            return *(Operable*)this;
        }

        Identifiable* cast_to_ident() override{
            return static_cast<Identifiable*>(this);
        }


        /**
         *
         * override assignable
         *
         * */
        // void assign_sim_value(ull b) override{
        //     mf_assert(get_assign_mode() == AM_SIM, "cannot assign in model mode");
        //     assert(_simEngine != nullptr);
        //     //////// TODO assign the value
        //     get_sim_engine_ptr()->get_proxy_rep().set_var(b);
        // }

        LogicComp<TYPE_COMP>& s(ull value){
            assert(is_cache_rep_init); ///// check from operable
            cached_rep.set_var(value);
            return *this;
        }

        LogicComp<TYPE_COMP>& s(ValRepBase value){
            assert(is_cache_rep_init);
            cached_rep.set_var(value);
            return *this;
        }

        void assign_sim_value(ull value) override{
            s(value);
        }

        void assign_sim_value(ValRepBase value) override{
            s(value);
        }

        Slice get_assign_slice() override{
            return Slicable<TYPE_COMP>::get_slice();
        }

        virtual void make_def_event(ull def_val) {assert(false);}

        void start_check_short_circuit(){
            Operable* match_opr = check_short_circuit();
            if (match_opr != nullptr){
                std::cout << get_md_ident_val() << "  match with " << match_opr->cast_to_ident()->get_ident_debug_value() << std::endl;
                mf_assert(false, "get short circuit");
            }
        }

        Assignable*get_assignable_from_assign_opr_ptr() override{
            return this;
        }

        Assignable*get_assignable_from_assign_callbacker_ptr() override{
            return this;
        }

    };

}

#endif //KATHRYN_LOGICCOMP_H
