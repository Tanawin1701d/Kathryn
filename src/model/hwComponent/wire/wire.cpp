//
// Created by tanawin on 29/11/2566.
//

#include "wire.h"
#include "model/hwComponent/expression/expression.h"
#include "model/controller/controller.h"


namespace kathryn{


    Wire::Wire(int size) : LogicComp({0, size},
                                     TYPE_WIRE,
                                     new WireLogicSim(this, size, VST_WIRE, false),
                                     true){
        com_init();
    }


    void Wire::com_init() {
        ctrl->on_wire_init(this);
    }

    Wire& Wire::operator=(Operable &b) {
        Slice absSlice = getSlice().getSubSliceWithShinkMsb({0, b.getOperableSlice().getSize()});
        ctrl->on_wire_update(generateBasicNode(b, absSlice), this);
        return *this;
    }

    Wire& Wire::operator=(Wire& b){
        if (this == &b){
            return *this;
        }

        operator=(*(Operable*)&b);

        return *this;
    }

    Wire& Wire::operator = (ull b){
        Operable& rhs = getMatchAssignOperable(b, getSlice().getSize());
        return operator=(rhs);
    }

    void Wire::generateAssMetaForNonBlocking(Operable& srcOpr,
                                       std::vector<AssignMeta*>& resultMetaCollector,
                                       Slice  absSrcSlice,
                                       Slice  absDesSlice){

        generateAssignMetaAndFill(srcOpr,resultMetaCollector,
                                  absSrcSlice,absDesSlice);

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

    Operable* Wire::doSlice(Slice sl){
        auto x = operator() (sl.start, sl.stop);
        return x.castToOperable();
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
        assert(absSliceOfHost.stop      <= getOperableSlice().stop);

        Slice absSlice = absSliceOfHost.getMatchSizeSubSlice(b.getOperableSlice());
        ctrl->on_wire_update(generateBasicNode(b, absSlice), this);
        return *this;
    }

    void Wire::callBackBlockAssignFromAgent(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                                            Slice absSrcSlice, Slice absDesSlice) {
        assert(getSlice().isContain(absDesSlice));
        generateAssMetaForBlocking(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice);
    }

    void Wire::callBackNonBlockAssignFromAgent(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                                               Slice absSrcSlice, Slice absDesSlice) {
        assert(getSlice().isContain(absDesSlice));
        generateAssMetaForNonBlocking(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice);
    }

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

    /**
     * Wire Logic Sim
     * */

    WireLogicSim::WireLogicSim(Wire* master,
                               int sz,
                               VCD_SIG_TYPE sigType,
                               bool simForNext):
            LogicSimEngine(sz, sigType, simForNext),
            _master(master){}

    void WireLogicSim::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        _master->assignValRepCurCycle(getCurVal());
    }

}