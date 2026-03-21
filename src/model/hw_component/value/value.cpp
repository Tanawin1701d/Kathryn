//
// Created by tanawin on 30/11/2566.
//

#include "value.h"

#include "utility"
#include "model/controller/controller.h"
#include "util/numberic/num_convert.h"
#include "sim/model_sim_engine/hw_component/value/value_sim.h"


namespace kathryn{

    Val::Val(int size, ull raw_value):
            LogicComp({0, size},
                      TYPE_VAL,
                      new ValSimEngine(this,VST_INTEGER, raw_value),
                      false),
            _size(size),
            _rawValue(raw_value)
    {
        assert(size > 0);
        com_init();
        AssignOpr::set_master(this);
        AssignCallbackFromAgent::set_master(this);
    }

    void Val::com_init() {
        ctrl->on_value_init(this);
    }

    SliceAgent<Val>& Val::operator()(int start, int stop){
        auto ret = new SliceAgent<Val>(
                this,
                get_abs_sub_slice(start, stop, get_slice())
                );
        return *ret;
    }

    SliceAgent<Val>& Val::operator() (int idx){
        return operator() (idx, idx+1);
    }

    SliceAgent<Val>& Val::operator() (Slice sl){
        return operator() (sl.start, sl.stop);
    }


    Operable* Val::do_slice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.cast_to_operable();
    }


    Operable* Val::check_short_circuit() {
        return nullptr;
    }

    void Val::create_logic_gen(){
        _genEngine = new ValueGen(
            _parent->get_module_gen_ptr(),
            this
        );
    }

}