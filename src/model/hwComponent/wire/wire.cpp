//
// Created by tanawin on 29/11/2566.
//

#include "wire.h"
#include "model/hwComponent/expression/expression.h"
#include "model/controller/controller.h"


namespace kathryn{


    Wire::Wire(int size,
        bool requireDefVal,
        bool initCom) : LogicComp({0, size},
                                     TYPE_WIRE,
                                     new WireSimEngine(this, VST_WIRE),
                                     true),
                                     _requireDefVal(requireDefVal)
    {
        if (initCom){
            com_init();
        }
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }


    void Wire::com_init() {
        ctrl->on_wire_init(this);
    }

    void Wire::doBlockAsm(Operable &srcOpr, Slice desSlice) {
        mfAssert(false, "wire doesn't support blocking asignment <<=");
    }

    void Wire::doNonBlockAsm(Operable &srcOpr, Slice desSlice) {
        assert(getAssignMode() == AM_MOD);
        Slice finalizeDesSlice = getSlice().getMatchSizeSubSlice(desSlice);
        ctrl->on_wire_update(
                generateBasicNode(srcOpr, finalizeDesSlice, ASM_DIRECT),
                this);
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

    SliceAgent<Wire>& Wire::operator()(Slice slc) {
        return operator() (slc.start,slc.stop);
    }

    Operable* Wire::doSlice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.castToOperable();
    }

    void Wire::makeDefEvent(){
        if (_requireDefVal){
            makeVal(defWireVal, getSlice().getSize(), 0);
            auto defEvent = new UpdateEvent({
                                                    nullptr,
                                                    nullptr,
                                                    &defWireVal,
                                                    {0, getSlice().getSize()},
                                                    DEFAULT_UE_PRI_MIN
                                            });

            addUpdateMeta(defEvent);
        }
    }

    /** override callback*/
    Operable *Wire::checkShortCircuit() {

        if (isInCheckPath){
            return this;
        }

        isInCheckPath = true;

        Operable* result = nullptr;

        for (auto upEvent: _updateMeta){
            assert(upEvent != nullptr);
            if (upEvent->srcUpdateValue != nullptr) {
                result = upEvent->srcUpdateValue->checkShortCircuit();
                if (result != nullptr) {
                    return result;
                }
            }
            if (upEvent->srcUpdateCondition != nullptr) {
                result = upEvent->srcUpdateCondition->checkShortCircuit();
                if (result != nullptr) {
                    return result;
                }
            }
            if(upEvent->srcUpdateState != nullptr){
                result =  upEvent->srcUpdateState->checkShortCircuit();
                if (result != nullptr){
                    return result;
                }
            }

        }

        isInCheckPath = false;
        return nullptr;
    }

    void Wire::createLogicGen(){
        assert(_parent->getModuleGen() != nullptr);

        _genEngine = new WireGen(
            _parent->getModuleGen(),
            {
                getType(),
                GEN_WIRE_GRP,
                getGlobalName(),
                -1,
                getOperableSlice(),
                false
            },
            this
        );
    }

    /**
     *
     *   wire sim engine
     *
     ***/
    WireSimEngine::WireSimEngine(Wire* master, VCD_SIG_TYPE sigType):
    LogicSimEngine(
        (Assignable*  ) master,
        (Identifiable*) master,
        VST_WIRE,
        false,
        0),
    _master(master){
        assert(_master != nullptr);
    }

}