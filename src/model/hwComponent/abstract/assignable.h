//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_ASSIGNABLE_H
#define KATHRYN_ASSIGNABLE_H

#include <vector>

#include "operable.h"
#include "model/hwComponent/abstract/operable.h"


namespace kathryn{

    /** reg/wire update meta data*/

    struct UpdateEvent{
        std::shared_ptr<Operable> updateCondition; /// which condition that allow this value to update.
        std::shared_ptr<Operable> updateState; /// which state that need to update.
        std::shared_ptr<Operable> updateValue; /// value to update.
        Slice                     updateSlice; /// slice to update
        int priority = 9;
        ///priority for circuit if there are attention to update same register at a time 0 is highest 9 is lowest
    };

    /**
     * Assignable represent hardware component that can memorize logic value or
     *
     * */

    template<typename RET_TYPE>
    class Assignable{
    protected:
        std::vector<UpdateEvent> _updateMeta;

    public:

        explicit Assignable(){};

        /** assignable value*/
        virtual RET_TYPE& operator <<= (Operable& b) = 0;
        virtual RET_TYPE& operator =   (Operable& b) = 0;

        /** update event management*/
        std::vector<UpdateEvent>& getUpdateMeta(){ return _updateMeta; }
        std::vector<UpdateEvent>& addUpdateMeta(const UpdateEvent& event){
            _updateMeta.push_back(event);
        }


    };

    template<typename RET_TYPE>
    class AssignCallbackFromAgent{
    public:
        virtual RET_TYPE& callBackBlockAssignFromAgent(Operable& b, Slice absSlice)  = 0;
        virtual RET_TYPE& callBackNonBlockAssignFromAgent(Operable& b, Slice absSlice)   = 0;
    };





}

#endif //KATHRYN_ASSIGNABLE_H
