//
// Created by tanawin on 24/1/2567.
//

#ifndef KATHRYN_UPDATEEVENT_H
#define KATHRYN_UPDATEEVENT_H

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
        int priority    = DEFAULT_UE_PRI_MIN;
        ///priority for circuit if there are attention to update same register at a time 0 is highest 9 is lowest
        ull subPriority = DEFAULT_UE_PRI_MIN; /// the value represent time when it is declared
                                              /// we the later assigment node should have higher priority to override older variable

        bool operator < (const UpdateEvent& rhs) const{
            if (priority < rhs.priority){
                return true;
            }if (priority == rhs.priority){
                if (subPriority < rhs.subPriority){
                    return true;
                }
                if (subPriority == rhs.subPriority){
                    if (srcUpdateValue->isConstOpr() && rhs.srcUpdateValue->isConstOpr()){
                        return srcUpdateValue->getConstOpr() < rhs.srcUpdateValue->getConstOpr();
                    }
                }
            }
            return false;
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