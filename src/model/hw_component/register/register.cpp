//
// Created by tanawin on 29/11/2566.
//

#include "register.h"

#include "model/hw_component/global_component/global_component.h"
#include "model/hw_component/expression/expression.h"
#include "model/controller/controller.h"
#include "sim/model_sim_engine/hw_component/register/register_sim.h"


namespace kathryn{

    /** constructor need to init communication with controller*/
    Reg::Reg(int size, bool init_com, HW_COMPONENT_TYPE hw_type, bool required_alloc_check) :
            LogicComp({0, size},
                      hw_type,
                      new RegSimEngine(this, VST_REG),
                      required_alloc_check){
        AssignOpr::set_master(this);
        AssignCallbackFromAgent::set_master(this);
        if (init_com) {
            com_init();
        }
        AssignOpr::set_master(this);
        AssignCallbackFromAgent::set_master(this);
    }

    void Reg::com_init() {
        ctrl->on_reg_init(this);
    }

    /***
     *
     * standard assignment
     *
     * */

    void Reg::do_block_asm(Operable&b, Slice des_slice) {
        do_global_asm(b, des_slice, ASM_DIRECT);
    }

    void Reg::do_non_block_asm(Operable&b, Slice des_slice){
        do_global_asm(b, des_slice, ASM_EQ_DEPNODE);
    }

    void Reg::do_global_asm(Operable& src_opr, Slice des_slice, ASM_TYPE asm_type) {
        assert(get_assign_mode() == AM_MOD);
        assert(des_slice.get_size() <= get_slice().get_size());
        assert(des_slice.stop <= get_slice().stop);
        /** bit control policy is shink the msb bit*/
        Slice finalize_des_slice = des_slice.get_match_size_sub_slice(src_opr.get_operable_slice());
        ctrl->on_reg_update(
                generate_basic_node(src_opr, finalize_des_slice, asm_type),
                this
        );
    }

    /** slicable override*/


    SliceAgent<Reg>& Reg::operator()(int start, int stop) {
        /***TODO slice_agent must be Delete
         * but fow now we neglect it
         * */
        auto ret =  new SliceAgent<Reg>(this,
                                        get_abs_sub_slice(start, stop, get_slice())
                                        );
        return *ret;
    }

    SliceAgent<Reg>& Reg::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    SliceAgent<Reg>& Reg::operator() (Slice sl){
        return operator() (sl.start, sl.stop);
    }


    Operable* Reg::do_slice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.cast_to_operable();
    }

    void Reg::make_reset_event(ull value, CLOCK_MODE cm){ //// we lock it to the posedge clock
        make_val(rst_reg_val, get_slice().get_size(), value);

        UpdateEventBase* ueb = create_ue_helper(nullptr,
                                              rst_wire,
                                              &rst_reg_val,
                                              {0, get_slice().get_size()},
                                              DEFAULT_UE_PRI_RST,
                                              cm,
                                              false);
        add_update_meta(ueb);
    }

    void Reg::make_def_event(ull def_value){
        make_val(rst_reg_val, get_slice().get_size(), def_value);

        UpdateEventBase* ueb = create_ue_helper(&rst_reg_val,
                                              {0, get_slice().get_size()},
                                              DEFAULT_UE_PRI_MIN,
                                              CM_POSEDGE, //// now it is lock for positive edge clock
                                              false);
        add_update_meta(ueb);

    }

    Operable* Reg::check_short_circuit(){
        return nullptr;
    }


    void Reg::create_logic_gen(){
        _genEngine = new RegGen(
            _parent->get_module_gen_ptr(),
            this
        );
    }

    bool Reg::check_integrity(){
        return get_marker() != WMT_GLOB_INPUT &&
               get_marker() != WMT_INPUT_MD   &&
               get_marker() != WMT_OUTPUT_MD;
    }

    Operable* Reg::get_opr_from_glob_io_ptr(){
        return this;
    };

    Assignable* Reg::get_asb_from_wire_marker_ptr(){
        return this;
    }

    /**
     * Reg Logic Sim
     * */



}