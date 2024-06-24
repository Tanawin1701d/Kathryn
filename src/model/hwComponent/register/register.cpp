//
// Created by tanawin on 29/11/2566.
//

#include "register.h"

#include "model/hwComponent/globalComponent/globalComponent.h"
#include "model/hwComponent/expression/expression.h"
#include "model/controller/controller.h"


namespace kathryn{

    /** constructor need to init communication with controller*/
    Reg::Reg(int size, bool initCom, HW_COMPONENT_TYPE hwType, bool requiredAllocCheck) :
            LogicComp({0, size},
                      hwType,
                      new RegSimEngine(this, VST_REG),
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

    /***
     *
     * standard assignment
     *
     * */

    void Reg::doBlockAsm(Operable&b, Slice desSlice) {
        doGlobalAsm(b, desSlice, ASM_DIRECT);
    }

    void Reg::doNonBlockAsm(Operable&b, Slice desSlice){
        doGlobalAsm(b, desSlice, ASM_EQ_DEPNODE);
    }

    void Reg::doGlobalAsm(Operable& srcOpr, Slice desSlice, ASM_TYPE asmType) {
        assert(getAssignMode() == AM_MOD);
        assert(desSlice.getSize() <= getSlice().getSize());
        assert(desSlice.stop <= getSlice().stop);
        /** bit control policy is shink the msb bit*/
        Slice finalizeDesSlice = desSlice.getMatchSizeSubSlice(srcOpr.getOperableSlice());
        ctrl->on_reg_update(
                generateBasicNode(srcOpr, finalizeDesSlice, asmType),
                this
        );
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

    SliceAgent<Reg>& Reg::operator() (Slice sl){
        return operator() (sl.start, sl.stop);
    }


    Operable* Reg::doSlice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.castToOperable();
    }

    void Reg::makeResetEvent(){
        makeVal(rstRegVal, getSlice().getSize(), 0);
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


    void Reg::createLogicGen(){
        _genEngine = new RegGen(
            _parent->getModuleGen(),
            {
                getType(),
                GEN_REG_GRP,
                getGlobalName(),
                -1,
                getOperableSlice(),
                false
            },
            this
        );
    }

    /**
     * Reg Logic Sim
     * */

    RegSimEngine::RegSimEngine(Reg* master,
                             VCD_SIG_TYPE sigType):
            LogicSimEngine(master, master, sigType, true, 0),
            _master(master){
        assert(master != nullptr);
    }

}