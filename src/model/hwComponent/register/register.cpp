//
// Created by tanawin on 29/11/2566.
//

#include "register.h"
#include "model/hwComponent/expression/expression.h"
#include "model/controller/controller.h"


namespace kathryn{

    /** constructor need to init communication with controller*/
    Reg::Reg(int size, bool initCom, HW_COMPONENT_TYPE hwType, bool requiredAllocCheck) :
            LogicComp({0, size},
                      hwType,
                      new RegLogicSim(this, size, VST_REG, true),
                      requiredAllocCheck){
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
        if (initCom) {
            com_init();
        }
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }

    void Reg::com_init() {
        ctrl->on_reg_init(this);
    }

    void Reg::doBlockAsm(Operable&b, Slice desSlice) {
        assert(getAssignMode() == AM_MOD);
        assert(desSlice.getSize() <= getSlice().getSize());
        assert(desSlice.stop <= getSlice().stop);
        /** bit control policy is shink the msb bit*/
        Slice finalizeDesSlice = desSlice.getMatchSizeSubSlice(b.getOperableSlice());
        ctrl->on_reg_update(
                generateBasicNode(b, finalizeDesSlice),
                this
        );
    }

    void Reg::doNonBlockAsm(Operable&b, Slice desSlice){
        mfAssert(false, "register doesn't support nonblocking assignment");
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

    Operable* Reg::doSlice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.castToOperable();
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

    Operable* Reg::checkShortCircuit(){
        return nullptr;
    }

    /**
     * Reg Logic Sim
     * */

    RegLogicSim::RegLogicSim(Reg* master,
                             int sz,
                             VCD_SIG_TYPE sigType,
                             bool simForNext):
            LogicSimEngine(sz, sigType, simForNext),
            _master(master){}

    void RegLogicSim::simStartCurCycle() {
        ///// if in This cycle the component is simmulated then skip simulation
        assert(isCurValSim());
    }

    void RegLogicSim::simStartNextCycle() {

        assert(isCurValSim());
        assert(!isNextValSim());

        setNextValSimStatus();
        getNextVal() = getCurVal(); ///// get curval to be next val because it may be no change
        _master->assignValRepCurCycle(getNextVal());
    }

}