//
// Created by tanawin on 24/6/2024.
//

#include "gen/proxy_hw_comp/wire/wire_auto_gen.h"
#include "model/hw_component/module/module.h"

namespace kathryn{

    WireAuto::WireAuto(int size, WIRE_AUTO_GEN_TYPE wire_io_type):
    Wire(size, false, false),
    _wireIoType(wire_io_type){}

    void WireAuto::build_hierarchy(Module* master_module){
        assert(master_module != nullptr);
        set_parent(master_module);
        build_inherit_name();
    }

    void WireAuto::create_logic_gen(){
        //// assert(_parent->get_module_gen_ptr() == nullptr);

        _genEngine = new WireAutoGen(
            _parent->get_module_gen_ptr(),
            this,
            _wireIoType
        );
    }


}