//
// Created by tanawin on 29/11/2566.
//

#include "wire.h"
#include "model/hwComponent/expression/expression.h"
#include "model/controller/controller.h"


namespace kathryn{


    Wire::Wire(int size) : LogicComp({0, size}, TYPE_WIRE, true){
        com_init();
    }


    void Wire::com_init() {
        ctrl->on_wire_init(this);
    }

    Wire& Wire::operator=(Operable &b) {
        Slice resultSlice = getSlice().getWeakAssignSlice({0, b.getOperableSlice().getSize()});
        ctrl->on_wire_update(generateAssignMeta(b, resultSlice), this);
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

    Wire& Wire::callBackBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        assert(true);
        return *this;
    }

    Wire& Wire::callBackNonBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        Slice resultSlice = absSliceOfHost.getWeakAssignSlice({0, b.getOperableSlice().getSize()});
        ctrl->on_wire_update(generateAssignMeta(b, resultSlice), this);
        return *this;
    }

    std::vector<std::string> Wire::getDebugAssignmentValue() {
        std::vector<std::string> results;
        for (auto upEvent: _updateMeta){
            results.push_back(upEvent->getDebugString());
        }
        return results;
    }


}