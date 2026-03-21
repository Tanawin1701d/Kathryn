//
// Created by tanawin on 4/8/2024.
//

#include "wire_gen.h"
#include "model/hw_component/wire/wire.h"


namespace kathryn{

    /**
     *
     * normal wire
     *
     */
    WireGen::WireGen(ModuleGen* md_gen_master,
                     Wire*      wire_master):
    AssignGenBase(md_gen_master,(Assignable*) wire_master, (Identifiable*) wire_master),
    _master(wire_master)
    { assert(_master != nullptr); }

    void WireGen::route_dep(){

        // if (_master->get_global_id() == 1701){
        //     std::cout<< "test";
        // }

        if (_master->get_marker() == WMT_INPUT_MD || _master->get_marker() == WMT_OUTPUT_MD){
            UpdatePool& master_update_pool = _master->get_update_meta();
            assert(master_update_pool.size() == 1);

            translated_update_pool = master_update_pool.clone();
            return;
        }
        /////// incase normal wire
        AssignGenBase::route_dep();

    }

    std::string WireGen::dec_io(){
        assert(_master->get_marker() == WMT_INPUT_MD || _master->get_marker() == WMT_OUTPUT_MD);

        std::string prefix;
        if (_master->get_marker() == WMT_INPUT_MD){
            prefix = "input ";
        }else if (_master->get_marker() == WMT_OUTPUT_MD){
            prefix = "output ";
        }else{
            assert(false);
        }
        Slice sl = _master->get_operable_slice();
        return  prefix + " wire[" + std::to_string(sl.stop-1) +": 0] " + get_opr();

    }
    std::string WireGen::dec_variable(){
        Slice sl = _master->get_operable_slice();
        std::string prefix_type;
        if(_master->get_marker() == WMT_INPUT_MD || _master->get_marker() == WMT_OUTPUT_MD){
            prefix_type = "wire";
        }else{
            prefix_type = "reg";
        }
        return prefix_type + " [" + std::to_string(sl.stop-1) +": 0] " +get_opr() + ";";
    }
    std::string WireGen::dec_op(){

        if(_master->get_marker() == WMT_INPUT_MD || _master->get_marker() == WMT_OUTPUT_MD){
            assert(!translated_update_pool.is_empty());
            UpdateEventBase* single_update_event = translated_update_pool.get_update_event_ref()[0];
            //assert(single_update_event->get_type() == UET_BASIC);

            CbBaseVerilog cb;
            UEBaseGenEngine* ue_gen_engine =  single_update_event->create_gen_engine();
            ue_gen_engine->gen_basic_connect(cb, this);
            delete ue_gen_engine;

            return cb.to_string(4);
        }

        return assign_op_base();
    }

}