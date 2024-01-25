//
// Created by tanawin on 24/1/2567.
//

#ifndef KATHRYN_UPDATEEVENT_H
#define KATHRYN_UPDATEEVENT_H

#include "model/simIntf/rtlSimEle.h"
#include "model/hwComponent/abstract/Slice.h"
#include "model/hwComponent/abstract/operable.h"
#include "operable.h"
#include <vector>

#endif //KATHRYN_UPDATEEVENT_H
namespace kathryn {
    /** reg/wire update metas data*/
    struct UpdateEvent{
        Operable* updateCondition = nullptr; /// which condition that allow this value to update.
        Operable* updateState     = nullptr; /// which state that need to update.
        Operable* updateValue     = nullptr; /// value to update.
        Slice     updateSlice; /// slice to update must smaller or equal to updateValue.slice
        int priority = 9;
        ///priority for circuit if there are attention to update same register at a time 0 is highest 9 is lowest

        bool operator < (const UpdateEvent& rhs) const{
            return priority < rhs.priority;
        }

        bool shouldAssignValRep(Operable* samplingOpr, bool isGetFromCur){
            if (samplingOpr != nullptr){
                RtlSimulatable* simItf = samplingOpr->castToRtlSimItf();
                assert(simItf != nullptr);
                auto simEnginePtr = simItf->getSimEngine();
                ValRep samplingVal = ValRep(1);
                if (isGetFromCur){
                    samplingVal = simEnginePtr->getCurVal();
                }else{
                    samplingVal = simEnginePtr->getBackVal();
                }
                if (!samplingVal.getLogicalValue()){
                    return false;
                }
            }
            return true;
        }

        void tryAssignValRep(ValRep& desValRep, bool getFromCur){
            ////// check update state valid
            if (!(shouldAssignValRep(updateCondition, getFromCur) &&
                  shouldAssignValRep(updateState, getFromCur))){
                return;
            }
            assert(updateValue != nullptr);

            ValRep& vr = getFromCur ? updateValue->castToRtlSimItf()->getSimEngine()->getCurVal()
                                    : updateValue->castToRtlSimItf()->getSimEngine()->getBackVal();
                    ;
            /**vr might have larger than desire updateSlice, so we must shink to match destination*/
            Slice vrSl = updateValue->getOperableSlice();
            vrSl = vrSl.getSubSliceWithShinkMsb({0, updateSlice.getSize()});
            ValRep actualVr = vr.slice(vrSl);
            /** update value */
            desValRep.updateOnSlice(actualVr, updateSlice);
        }

        void trySimAll() const{
            trySim(updateCondition);
            trySim(updateState);
            trySim(updateValue);
        }

        static void trySim(Operable* opr){
            if (opr != nullptr)
                opr->castToRtlSimItf()->simStartCurCycle();
        }

        [[nodiscard]] std::string getDebugString() const{
            return updateValue->castToIdent()->getGlobalName() +
            "[" +
            std::to_string(updateSlice.start) + ":"+
            std::to_string(updateSlice.stop) +
            "] when state = " +
            ((updateState != nullptr) ? updateState->castToIdent()->getGlobalName(): "none") +
            " cond = " +
            ((updateCondition != nullptr) ? updateCondition->castToIdent()->getGlobalName(): "none");
        }
    };
}