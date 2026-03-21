//
// Created by tanawin on 29/11/2566.
//

#include "wire.h"
#include "wire_auto.h"
#include "model/hw_component/expression/expression.h"
#include "model/controller/controller.h"
#include "sim/model_sim_engine/hw_component/wire/wire_sim.h"


namespace kathryn{


    Wire::Wire(int size,
        bool require_def_val,
        bool init_com) : LogicComp({0, size},
                                     TYPE_WIRE,
                                     new WireSimEngine(this, VST_WIRE),
                                     true),
                                     _requireDefVal(require_def_val)
    {
        if (init_com){
            com_init();
        }
        AssignOpr::set_master(this);
        AssignCallbackFromAgent::set_master(this);
    }


    void Wire::com_init() {
        ctrl->on_wire_init(this);
    }

    void Wire::do_block_asm(Operable &src_opr, Slice des_slice) {
        mf_assert(false, "wire doesn't support blocking asignment <<=");
    }

    void Wire::do_non_block_asm(Operable &src_opr, Slice des_slice) {
        assert(get_assign_mode() == AM_MOD);
        assert(get_slice().is_contain(des_slice));
        Slice finalize_des_slice = des_slice.get_match_size_sub_slice(src_opr.get_operable_slice());
        ctrl->on_wire_update(
                generate_basic_node(src_opr, finalize_des_slice, ASM_DIRECT),
                this);
    }

    SliceAgent<Wire>& Wire::operator()(int start, int stop) {
        auto ret = new SliceAgent<Wire>(
                this,
                get_abs_sub_slice(start, stop, get_slice())
                        );
        return *ret;
    }

    SliceAgent<Wire>& Wire::operator()(int idx) {
        return operator()(idx, idx+1);
    }

    SliceAgent<Wire>& Wire::operator()(Slice slc) {
        return operator() (slc.start,slc.stop);
    }

    Operable* Wire::do_slice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.cast_to_operable();
    }

    void Wire::make_def_event(ull def_val){
        if (_requireDefVal){
            make_val(def_wire_val, get_slice().get_size(), def_val);

            UpdateEventBasic*  def_event = create_ue_helper(&def_wire_val,
                                                         {0, get_slice().get_size()},
                                                         DEFAULT_UE_PRI_MIN,
                                                         CM_CLK_FREE,
                                                         false);
            add_update_meta(def_event);
        }
    }

    /** override callback*/
    Operable* Wire::check_short_circuit() {

        if (is_in_check_path){
            return this;
        }

        is_in_check_path = true;

        Operable* result = get_update_meta().check_short_circuit_proxy();

        is_in_check_path = false;
        return nullptr;
    }

    void Wire::create_logic_gen(){
        assert(_parent->get_module_gen_ptr() != nullptr);

        _genEngine = new WireGen(
            _parent->get_module_gen_ptr(),
            this
        );
    }

    /** override global input*/
    bool Wire::check_integrity(){
        return true;
    }
    Operable*   Wire::get_opr_from_glob_io_ptr(){ return this;}
    Assignable* Wire::get_asb_from_wire_marker_ptr(){ return this;}


    /////// global input pool




}