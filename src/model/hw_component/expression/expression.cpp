//
// Created by tanawin on 28/11/2566.
//

#include "expression.h"
#include "model/controller/controller.h"
#include "sim/model_sim_engine/hw_component/expression/expression_sim.h"

namespace kathryn{

    /**
     * expr_metas
     * */

    expression::expression(LOGIC_OP op,
                           const Operable* a,
                           const Operable* b,
                           int exp_size):
    LogicComp<expression>({0, exp_size},
                          TYPE_EXPRESSION,
                          new expression_sim_engine(this, VST_WIRE),
                          false),
    _valueAssinged(true),
    _op(op),
    _a(const_cast<Operable *>(a)),
    _b(const_cast<Operable *>(b))
    {
        com_init();
        AssignOpr::set_master(this);
        AssignCallbackFromAgent::set_master(this);
    }

    expression::expression(int exp_size):
    LogicComp<expression>({0, exp_size},
                          TYPE_EXPRESSION,
                          new expression_sim_engine(this, VST_WIRE),
                          false),
    _valueAssinged(false),
    _op(ASSIGN),
    _a(nullptr),
    _b(nullptr){
        com_init();
        AssignOpr::set_master(this);
        AssignCallbackFromAgent::set_master(this);
    }


    void expression::com_init() {
        ctrl->on_expression_init(this);
    }

    void expression::do_non_block_asm(Operable &src_opr, Slice des_slice) {
        assert(!_valueAssinged);
        do_non_block_asm_mul_ass_check(src_opr, des_slice);
    }

    void expression::do_non_block_asm_mul_ass_check(Operable& src_opr, Slice des_slice){
        mf_assert(get_assign_mode() == AM_MOD, "expression can use operator = only in MD mode");
        _a = &src_opr;
        assert(src_opr.get_operable_slice().get_size() == get_operable_slice().get_size());
        assert(des_slice.get_size() == get_operable_slice().get_size());
        mf_assert(!_valueAssinged, "multiple expression assign detect");
        _valueAssinged = true;
    }



    SliceAgent<expression>& expression::operator()(int start, int stop) {
        auto ret =  new SliceAgent<expression>(this,
                                               get_abs_sub_slice(start, stop, get_slice()));
        return *ret;
    }

    SliceAgent<expression>& expression::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    SliceAgent<expression>& expression::operator()(Slice sl) {
        return operator() (sl.start, sl.stop);
    }

    Operable* expression::do_slice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.cast_to_operable();
    }

    Operable* expression::check_short_circuit(){
        if (is_in_check_path){
            std::cout << "path end " << std::to_string(cast_to_ident()->get_global_id()) << std::endl;
            return this;
        }
        is_in_check_path = true;

        Operable* result;
        if (_a != nullptr){
            result = _a->check_short_circuit();
            if (result != nullptr){
                std::cout << "path a " << std::to_string(cast_to_ident()->get_global_id()) << std::endl;
                return result;
            }
        }
        if (_b != nullptr){
            result = _b->check_short_circuit();
            if (result != nullptr){
                std::cout << "path b " << std::to_string(cast_to_ident()->get_global_id()) << std::endl;
                return result;
            }
        }

        is_in_check_path = false;
        return nullptr;
    }

    void expression::create_logic_gen(){
        _genEngine = new ExprGen(
            _parent->get_module_gen_ptr(),
            this
        );
    }





}