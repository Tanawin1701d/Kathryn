//
// Created by tanawin on 29/11/2566.
//

#include "register.h"
#include "model/hwComponent/expression/expression.h"
#include "model/controller/controller.h"


namespace kathryn{

    /** constructor need to init communication with controller*/
    Reg::Reg(int size, bool initCom, HW_COMPONENT_TYPE hwType, bool requiredAllocCheck) :
            LogicComp({0, size}, hwType,
                      requiredAllocCheck){
        if (initCom) {
            com_init();
        }
    }

    void Reg::com_init() {
        ctrl->on_reg_init(this);
    }

    Reg& Reg::operator<<=(Operable &b) {
        Slice absSlice = getSlice().getWeakAssignSlice({0, b.getOperableSlice().getSize()});
        ctrl->on_reg_update(generateAssignMeta(b, absSlice), this);
        return *this;
    }

    Reg& Reg::operator=(Operable &b) {
        /** todo first version we not support this operator*/
        assert(false);
    }

    /** slicable override*/


    SliceAgent<Reg>& Reg::operator()(int start, int stop) {
        /***TODO sliceAgent must be Delete
         * but fow now we neglect it
         * */
        auto ret =  new SliceAgent<Reg>(this,
                                        getNextSlice(start, stop, getSlice())
                                        );
        return *ret;
    }

    SliceAgent<Reg>& Reg::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    Reg& Reg::callBackBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        Slice resultSlice = absSliceOfHost.getWeakAssignSlice({0, b.getOperableSlice().getSize()});
        ctrl->on_reg_update(generateAssignMeta(b, resultSlice), this);
        return *this;
    }

    Reg &Reg::callBackNonBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        assert(false);
    }

    void Reg::simStartCurCycle() {
        ///// if in This cycle the component is simmulated then skip simulation
        if (isCurCycleSimulated()){
            return;
        }
        setSimStatus();
        assignValRepCurCycle(getSimEngine()->getCurVal(), false);
        //// we assign false because it is register, we must get from back cycle
    }

    void Reg::simExitCurCycle() {
            resetSimStatus();
            getSimEngine()->iterate();
    }

//    std::vector<std::string> Reg::getDebugAssignmentValue() {
//        std::vector<std::string> results;
//        for (auto upEvent: _updateMeta){
//            assert(upEvent != nullptr);
//            results.push_back(upEvent->getDebugString());
//        }
//        return results;
//    }


}