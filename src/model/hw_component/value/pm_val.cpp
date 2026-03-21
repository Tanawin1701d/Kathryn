//
// Created by tanawin on 14/1/2025.
//

#include "pm_val.h"
#include "utility"
#include "model/controller/controller.h"
#include "sim/model_sim_engine/hw_component/value/pm_value_sim.h"
#include "gen/proxy_hw_comp/value/param_value_gen.h"


namespace kathryn{


    PmVal::PmVal(ull raw_value):
        LogicComp({0, _size},
            TYPE_PMVAL,
            new PmValSimEngine(this, VST_INTEGER),
            false),
        _size(64),
        _rawValue(raw_value)
    {
        com_init();
        AssignOpr::set_master(this);
        AssignCallbackFromAgent::set_master(this);
    }

    void PmVal::com_init(){
        //// to do add to controller
        ctrl->on_pmValue_init(this);
    }

    SliceAgent<PmVal>& PmVal::operator()(int start, int stop){
        auto ret = new SliceAgent<PmVal>(
                    this,
                    get_abs_sub_slice(start, stop, get_slice())
            );
        return *ret;
    }

    SliceAgent<PmVal>& PmVal::operator() (int idx){
        return operator() (idx, idx+1);
    }

    SliceAgent<PmVal>& PmVal::operator() (Slice sl){
        return operator() (sl.start, sl.stop);
    }

    Operable* PmVal::do_slice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.cast_to_operable();
    }

    Operable* PmVal::check_short_circuit() {
        return nullptr;
    }

    void PmVal::create_logic_gen(){
        ////// todo create logic gen
        _genEngine = new ParamValGen(_parent->get_module_gen_ptr(), this);

    }

}