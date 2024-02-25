//
// Created by tanawin on 25/2/2567.
//

#include "nest.h"
#include "model/controller/controller.h"

namespace kathryn{


    /** pre function declaration*/

    std::vector<Operable*> getNestVec(){
        return {};
    }



    /** nest class*/

    void nest::com_init() {
        ctrl->on_nest_init(this);
    }

    nest::nest(int size, std::vector<Operable *> nestList):
    LogicComp<nest>({0, size}, TYPE_NEST, VST_WIRE, false, false),
    _nestList(std::move(nestList))
    {
        int testSize = 0;
        for (Operable* opr: _nestList){
            assert(opr != nullptr);
            testSize += opr->getOperableSlice().getSize();
        }
        assert(testSize == size);
        com_init();
    }

    SliceAgent<nest> &nest::operator()(int start, int stop) {
        auto ret =  new SliceAgent<nest>(this,
                                               getAbsSubSlice(start, stop, getSlice()));
        return *ret;
    }

    SliceAgent<nest> &nest::operator()(int idx) {
        return operator()(idx, idx + 1);
    }

    nest& nest::callBackBlockAssignFromAgent(Operable &b, Slice absSlice) {
        assert(false);
    }

    nest& nest::callBackNonBlockAssignFromAgent(Operable &b, Slice absSlice) {
        assert(false);
    }


    Operable* nest::checkShortCircuit() {
        if (isInCheckPath){
            return this;
        }

        isInCheckPath = true;

        for (Operable* hostOpr: _nestList){
            Operable* result = hostOpr->checkShortCircuit();
            if (result != nullptr){
                return result;
            }
        }

        isInCheckPath = false;
        return nullptr;
    }

    void nest::simStartCurCycle() {
        if (getRtlValItf()->isCurValSim()){
            return;
        }
        getRtlValItf()->setCurValSimStatus();

        /**get sim value and reset it*/
        ValRep& desValRep = getRtlValItf()->getCurVal();
        desValRep.fillZeroToValrep(0);

        int curStartBit = 0;

        for (Operable* opr: _nestList){
            /**slave opr first*/
            assert(opr != nullptr);
            opr->getSimItf()->simStartCurCycle();
            assert(opr->getRtlValItf()->isCurValSim());
            ValRep oprVal = opr->getSlicedCurValue();
            assert(oprVal.getLen() == opr->getOperableSlice().getSize());

            ///std::cout << oprVal.getBiStr() << std::endl;

            /** cal next opr*/
            desValRep.updateOnSlice(oprVal,
                                    {curStartBit,
                                           curStartBit + opr->getOperableSlice().getSize()}
                                           );
            curStartBit += opr->getOperableSlice().getSize();

        }
        assert(curStartBit == getSlice().getSize());


    }


}