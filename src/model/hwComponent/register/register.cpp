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
        if (initCom) {
            com_init();
        }
    }

    void Reg::com_init() {
        ctrl->on_reg_init(this);
    }

    Reg& Reg::operator<<=(Operable &b) {
        assert(getAssignMode() == AM_MOD);
        Slice absSlice = getSlice().getSubSliceWithShinkMsb({0, b.getOperableSlice().getSize()});
        ctrl->on_reg_update(generateBasicNode(b, absSlice), this);
        return *this;
    }

    Reg& Reg::operator <<= (ull b) {
        assert(getAssignMode() == AM_MOD);
        Operable& rhs = getMatchAssignOperable(b, getSlice().getSize());
        return operator<<=(rhs);
    }

    void Reg::generateAssMetaForBlocking(Operable& srcOpr,
                                         std::vector<AssignMeta*>& resultMetaCollector,
                                         Slice  absSrcSlice,
                                         Slice  absDesSlice) {
        generateAssignMetaAndFill(srcOpr,
                                  resultMetaCollector,
                                  absSrcSlice,
                                  absDesSlice);
    }

    Reg& Reg::operator=(Operable& b) {
        /** todo first version we not support this operator*/
        mfAssert(false, "reg don't support this = assigment");
        assert(false);
    }

    Reg& Reg::operator=(ull b){
        assert(getAssignMode() == AM_SIM);
        assignSimValue(b);
        return *this;
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

    Operable* Reg::doSlice(Slice sl){
        auto x = operator() (sl.start, sl.stop);
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

    Reg& Reg::callBackBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        assert(absSliceOfHost.getSize() <= getOperableSlice().getSize());
        assert(absSliceOfHost.stop      <= getOperableSlice().stop);
        Slice absSlice = absSliceOfHost.getSubSliceWithShinkMsb({0, b.getOperableSlice().getSize()});
        ctrl->on_reg_update(generateBasicNode(b, absSlice), this);
        return *this;
    }

    Reg &Reg::callBackNonBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        assert(false);
    }

    void Reg::callBackBlockAssignFromAgent(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                                            Slice absSrcSlice, Slice absDesSlice) {
        assert(getSlice().isContain(absDesSlice));
        generateAssMetaForBlocking(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice);
    }

    void Reg::callBackNonBlockAssignFromAgent(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                                               Slice absSrcSlice, Slice absDesSlice) {
        assert(getSlice().isContain(absDesSlice));
        generateAssMetaForNonBlocking(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice);
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