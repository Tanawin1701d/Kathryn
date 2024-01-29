//
// Created by tanawin on 29/11/2566.
//

#include "wire.h"
#include "model/hwComponent/expression/expression.h"
#include "model/controller/controller.h"


namespace kathryn{


    Wire::Wire(int size) : LogicComp({0, size}, TYPE_WIRE,
                                     new RtlSimEngine(size, VST_WIRE, false),true){
        com_init();
    }


    void Wire::com_init() {
        ctrl->on_wire_init(this);
    }

    Wire& Wire::operator=(Operable &b) {
        Slice absSlice = getSlice().getSubSliceWithShinkMsb({0, b.getOperableSlice().getSize()});
        ctrl->on_wire_update(generateAssignMeta(b, absSlice), this);
        return *this;
    }

    SliceAgent<Wire>& Wire::operator()(int start, int stop) {
        auto ret = new SliceAgent<Wire>(
                this,
                getAbsSubSlice(start, stop, getSlice())
                        );
        return *ret;
    }

    SliceAgent<Wire>& Wire::operator()(int idx) {
        return operator()(idx, idx+1);
    }

    void Wire::makeDefEvent(){
        makeVal(defWireVal, genBiConValRep(0, getSlice().getSize()));
        auto defEvent = new UpdateEvent({
                                                nullptr,
                                                nullptr,
                                                &defWireVal,
                                                {0, getSlice().getSize()},
                                                DEFAULT_UE_PRI_MIN
                                        });
        addUpdateMeta(defEvent);
    }

    /** override callback*/

    Wire& Wire::callBackBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        assert(false);
    }

    Wire& Wire::callBackNonBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        ///Slice absSlice = absSliceOfHost.getSubSliceWithShinkMsb({0, b.getOperableSlice().getSize()});
        assert(absSliceOfHost.getSize() <= getOperableSlice().getSize());
        assert(absSliceOfHost.getSize() <= b.getOperableSlice().getSize());
        ctrl->on_wire_update(generateAssignMeta(b, absSliceOfHost), this);
        return *this;
    }

    void Wire::simStartCurCycle() {

        if (getSimEngine()->isCurValSim()){
            return;
        }
        getSimEngine()->setCurValSimStatus();
        assignValRepCurCycle(getSimEngine()->getCurVal());

    }






//    std::vector<std::string> Wire::getDebugAssignmentValue() {
//        std::vector<std::string> results;
//        for (auto upEvent: _updateMeta){
//            results.push_back(upEvent->getDebugString());
//        }
//        return results;
//    }


}