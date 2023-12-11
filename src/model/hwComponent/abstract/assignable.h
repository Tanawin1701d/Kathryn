//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_ASSIGNABLE_H
#define KATHRYN_ASSIGNABLE_H

#include <vector>

#include "operable.h"
#include "model/hwComponent/abstract/operable.h"


namespace kathryn{

    /** reg/wire update metas data*/

    struct UpdateEvent{
        Operable* updateCondition{}; /// which condition that allow this value to update.
        Operable* updateState{}; /// which state that need to update.
        Operable* updateValue{}; /// value to update.
        Slice     updateSlice; /// slice to update
        int priority = 9;
        ///priority for circuit if there are attention to update same register at a time 0 is highest 9 is lowest

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

    /* This is used to describe what and where to update that send to controller and let flow block determine*/
    struct AssignMeta{
        std::vector<UpdateEvent*>& updateEventsPool;
        Operable& valueToAssign;
        Slice desSlice;
        AssignMeta(std::vector<UpdateEvent*>& u, Operable& v, Slice s): updateEventsPool(u),
                                                                        valueToAssign(v),
                                                                        desSlice(s){}
    };

    /**
    * Assignable represent hardware component that can memorize logic value or
    *
    * */

    template<typename RET_TYPE>
    class Assignable{
    protected:
        std::vector<UpdateEvent*> _updateMeta;
    public:

        explicit Assignable() = default;
        ~Assignable(){
            for (auto eventPtr: _updateMeta){
                delete eventPtr;
            }
        }

        /** assignable value*/
        virtual RET_TYPE& operator <<= (Operable& b) = 0;
        virtual RET_TYPE& operator =   (Operable& b) = 0;

        /** update event management*/
        std::vector<UpdateEvent*>& getUpdateMeta(){ return _updateMeta; }
        void addUpdateMeta(UpdateEvent* event){
            _updateMeta.push_back(event);
        }
        /** generate update metas*/
        AssignMeta* generateAssignMeta(Operable& assignValue, Slice assignSlice){
            return new AssignMeta(_updateMeta, assignValue, assignSlice);
        }

    };


    template<typename RET_TYPE>
    class AssignCallbackFromAgent{
    public:
        virtual RET_TYPE& callBackBlockAssignFromAgent(Operable& b, Slice absSlice)    = 0;
        virtual RET_TYPE& callBackNonBlockAssignFromAgent(Operable& b, Slice absSlice) = 0;
    };
}

#endif //KATHRYN_ASSIGNABLE_H
