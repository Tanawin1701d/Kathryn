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
        ctrl->on_wire_init(std::shared_ptr<Wire>(this));
    }

    Wire& Wire::operator=(Operable &b) {
        /** todo communicate with commustack to assign */
        return *this;
    }

    SliceAgent<Wire>& Wire::operator()(int start, int stop) {
        auto ret = std::make_shared<SliceAgent<Wire>>(
                            std::shared_ptr<Wire>(this),
                            getNextSlice(start, stop, getSlice())
                            );
        return *ret;
    }

    SliceAgent<Wire> &Wire::operator()(int idx) {
        return operator()(idx, idx+1);
    }

    /** override callback*/

    Wire &Wire::callBackBlockAssignFromAgent(Operable &b, Slice absSlice) {
        /** todo this must call model control system to determine
        * given information and condition of updating value
        /* we will call model building to comunicate with it*/
        //** todo return agent of this type*/
        return *this;
    }

    Wire &Wire::callBackNonBlockAssignFromAgent(Operable &b, Slice absSlice) {
        return *this;
    }






}