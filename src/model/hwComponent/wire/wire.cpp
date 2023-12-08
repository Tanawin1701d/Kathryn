//
// Created by tanawin on 29/11/2566.
//

#include "wire.h"
#include "model/hwComponent/expression/expression.h"
#include "model/controller/controller.h"


namespace kathryn{


    Wire::Wire(int size) : Assignable(), Operable(),
                           Slicable(Slice{0, size}),
                           AssignCallbackFromAgent<Wire>(),
                           Identifiable(TYPE_WIRE),
                           HwCompControllerItf(){
        com_init();
    }


    void Wire::com_init() {
        ctrl->on_wire_init(this);
    }

    Wire& Wire::operator=(Operable &b) {
        ctrl->on_wire_update(generateAssignMeta(b, getSlice()));
        return *this;
    }

    SliceAgent<Wire>& Wire::operator()(int start, int stop) {
        auto ret = new SliceAgent<Wire>(
                        this,
                        getNextSlice(start, stop, getSlice())
                        );
        return *ret;
    }

    SliceAgent<Wire>& Wire::operator()(int idx) {
        return operator()(idx, idx+1);
    }

    /** override callback*/

    Wire& Wire::callBackBlockAssignFromAgent(Operable &b, Slice absSlice) {
        assert(true);
    }

    Wire& Wire::callBackNonBlockAssignFromAgent(Operable &b, Slice absSlice) {
        ctrl->on_wire_update(generateAssignMeta(b, absSlice));
        return *this;
    }






}