//
// Created by tanawin on 29/11/2566.
//

#include "register.h"
#include "model/hwComponent/expression/expression.h"
#include "model/controller/controller.h"


namespace kathryn{

    /** constructor need to init communication with controller*/
    Reg::Reg(int size, bool initCom, HW_COMPONENT_TYPE hwType, bool requiredAllocCheck) :
            LogicComp({0, size}, hwType,VST_REG, true,requiredAllocCheck){
        if (initCom) {
            com_init();
        }
    }

    void Reg::com_init() {
        ctrl->on_reg_init(this);
    }

    Reg& Reg::operator<<=(Operable &b) {
        Slice absSlice = getSlice().getSubSliceWithShinkMsb({0, b.getOperableSlice().getSize()});
        ctrl->on_reg_update(generateAssignMeta(b, absSlice), this);
        return *this;
    }

    Reg& Reg::operator <<= (ull b) {
        Operable& rhs = getMatchAssignOperable(b, getSlice().getSize());
        return operator<<=(rhs);
    }

    Reg& Reg::operator=(Operable& b) {
        /** todo first version we not support this operator*/
        mfAssert(false, "reg don't support this = assigment");
        assert(false);
    }

    Reg& Reg::operator=(Reg& b){
        assert(false);
    }

    /** slicable override*/


    SliceAgent<Reg>& Reg::operator()(int start, int stop) {
        /***TODO sliceAgent must be Delete
         * but fow now we neglect it
         * */
        auto ret =  new SliceAgent<Reg>(this,
                                        getAbsSubSlice(start, stop, getSlice())
                                        );
        return *ret;
    }

    SliceAgent<Reg>& Reg::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    void Reg::makeResetEvent(){
        makeVal(rstRegVal, genBiConValRep(0, getSlice().getSize()));
        auto rstEvent = new UpdateEvent({
            nullptr,
            rstWire,
            &rstRegVal,
            {0, getSlice().getSize()},
            DEFAULT_UE_PRI_RST
        });
        addUpdateMeta(rstEvent);
    }

    Reg& Reg::callBackBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        assert(absSliceOfHost.getSize() <= getOperableSlice().getSize());
        assert(absSliceOfHost.stop      <= getOperableSlice().stop);
        Slice absSlice = absSliceOfHost.getSubSliceWithShinkMsb({0, b.getOperableSlice().getSize()});
        ctrl->on_reg_update(generateAssignMeta(b, absSlice), this);
        return *this;
    }

    Reg &Reg::callBackNonBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        assert(false);
    }

    void Reg::simStartCurCycle() {
        ///// if in This cycle the component is simmulated then skip simulation
        assert(getRtlValItf()->isCurValSim());
    }

    void Reg::simStartNextCycle() {

        RtlValItf* rtlValItf = getRtlValItf();
        assert(rtlValItf->isCurValSim());
        assert(!rtlValItf->isNextValSim());

        rtlValItf->setNextValSimStatus();
        rtlValItf->getNextVal() = rtlValItf->getCurVal(); ///// get curval to be next val because it may be no change
        assignValRepCurCycle(rtlValItf->getNextVal());
    }

    Operable* Reg::checkShortCircuit(){
        return nullptr;
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