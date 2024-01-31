//
// Created by tanawin on 24/1/2567.
//

#ifndef KATHRYN_UPDATEEVENT_H
#define KATHRYN_UPDATEEVENT_H

#include "model/simIntf/modelSimEngine.h"
#include "model/hwComponent/abstract/Slice.h"
#include "model/hwComponent/abstract/operable.h"
#include "operable.h"
#include <vector>

#endif //KATHRYN_UPDATEEVENT_H
namespace kathryn {

    /** default -->update -->event -->priority */
    static int DEFAULT_UE_PRI_USER     = 10;
    static int DEFAULT_UE_PRI_INTERNAL_MAX = 100;
    static int DEFAULT_UE_PRI_INTERNAL_MIN = 50;
    static int DEFAULT_UE_PRI_RST      = INT32_MAX;
    static int DEFAULT_UE_PRI_MIN      = 0;

/** reg/wire update metas data*/
    struct UpdateEvent{
        Operable* srcUpdateCondition = nullptr; /// which condition that allow this value to update.
        Operable* srcUpdateState     = nullptr; /// which state that need to update.
        Operable* srcUpdateValue     = nullptr; /// value to update.
        Slice     desUpdateSlice; /// slice to update must smaller or equal to srcUpdateValue.slice
        int priority = DEFAULT_UE_PRI_MIN;
        ///priority for circuit if there are attention to update same register at a time 0 is highest 9 is lowest

        bool operator < (const UpdateEvent& rhs) const{
            return priority < rhs.priority;
        }

        bool shouldAssignValRep(Operable* opr){
            if (opr != nullptr){
                assert(opr->castToRtlSimItf()->getSimEngine()->isCurValSim());
                ValRep samplingVal           = opr->getExactSimCurValue().slice(opr->getOperableSlice());
                return samplingVal.getLogicalValue();
            }
            return true;
        }

        void tryAssignValRep(ValRep& desValRep){
            ////// check update state valid
            if (!(shouldAssignValRep(srcUpdateCondition) &&
                  shouldAssignValRep(srcUpdateState))){
                return;
            }
            /**clarify slice*/
            Slice desSlice = desUpdateSlice;
            Slice srcSlice = srcUpdateValue->getOperableSlice();
                  assert(srcSlice.getSize() >= desSlice.getSize());
                  srcSlice = srcSlice.getSubSliceWithShinkMsb({0, desSlice.getSize()});
                  assert(srcSlice.getSize() == desSlice.getSize());
            /**get src value and slice */
            ValRep& srcSimVal = srcUpdateValue->getExactSimCurValue();
            assert(srcUpdateValue->castToRtlSimItf()->getSimEngine()->isCurValSim());
            ValRep  srcSimValFit = srcSimVal.slice(srcSlice);
            desValRep.updateOnSlice(srcSimValFit, desSlice);
        }

        void trySimAll() const{
            trySim(srcUpdateCondition);
            trySim(srcUpdateState);
            trySim(srcUpdateValue);
        }

        static void trySim(Operable* opr){
            if (opr != nullptr)
                opr->castToRtlSimItf()->simStartCurCycle();
                ///////// get exact operable due to it may be agent
        }

        [[nodiscard]] std::string getDebugString() const{
            return srcUpdateValue->castToIdent()->getGlobalName() +
                   "[" +
                   std::to_string(desUpdateSlice.start) + ":" +
                   std::to_string(desUpdateSlice.stop) +
                   "] when state = " +
                   ((srcUpdateState != nullptr) ? srcUpdateState->castToIdent()->getGlobalName() : "none") +
                   " cond = " +
                   ((srcUpdateCondition != nullptr) ? srcUpdateCondition->castToIdent()->getGlobalName() : "none");
        }
    };
}