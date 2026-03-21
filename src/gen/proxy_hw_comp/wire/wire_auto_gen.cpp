//
// Created by tanawin on 20/6/2024.
//

#include "wire_auto_gen.h"
#include "model/hw_component/wire/wire_auto.h"

namespace kathryn{

    WireAutoGen::WireAutoGen(
        ModuleGen*          md_gen_master,
        WireAuto*           wire_master,
        WIRE_AUTO_GEN_TYPE  auto_wire_gen_type
    ):
    AssignGenBase(md_gen_master,(Assignable*) wire_master, (Identifiable*) wire_master),
     _autoWireGenType(auto_wire_gen_type),
    _master(wire_master){}

    std::string WireAutoGen::get_opr(){
        if ((_autoWireGenType == WIRE_AUTO_GEN_GLOB_INPUT) ||
            (_autoWireGenType == WIRE_AUTO_GEN_GLOB_OUTPUT)){
            return _master->get_var_name();
        }
        return LogicGenBase::get_opr();
    }


    std::string WireAutoGen::dec_io(){
        assert(_autoWireGenType != WIRE_AUTO_GEN_INTER);
        Slice sl = _master->get_operable_slice();
        return std::string((_autoWireGenType == WIRE_AUTO_GEN_INPUT || _autoWireGenType == WIRE_AUTO_GEN_GLOB_INPUT) ? "input" : "output") +
        " wire[" + std::to_string(sl.stop-1) +": 0] " + get_opr();
    }

    std::string WireAutoGen::dec_variable(){
        Slice sl = _master->get_operable_slice();
        return "wire [" + std::to_string(sl.stop-1) +": 0] " +get_opr() + ";";
    }

    std::string WireAutoGen::dec_op(){
        assert(!translated_update_pool.is_empty());
        UpdateEventBase* single_update_event = translated_update_pool.get_update_event_ref()[0];
        assert(single_update_event != nullptr);
        assert(single_update_event->get_type() == UET_BASIC);

        CbBaseVerilog cb;
        UEBaseGenEngine* ue_gen_engine = single_update_event->create_gen_engine();
        ue_gen_engine->gen_basic_connect(cb, this);
        delete ue_gen_engine;

        std::string result = cb.to_string(4);

        return result;
    }


}