//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_ASSIGNABLE_H
#define KATHRYN_ASSIGNABLE_H

#include <vector>
#include <algorithm>

#include "operable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/Slice.h"
#include "model/simIntf/modelSimEngine.h"
#include "updateEvent.h"

namespace kathryn{


    /** This is used to describe what and where to update that send to controller and let flow block determine*/
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
    /** to make value when constant input is used*/
    Operable& getMatchAssignOperable(ull value, const int size);

    template<typename RET_TYPE>
    class Assignable{
    protected:
        std::vector<UpdateEvent*> _updateMeta;
    public:

        explicit Assignable() = default;
        virtual ~Assignable(){
            for (auto eventPtr: _updateMeta){
                delete eventPtr;
            }
        }

        /** assignable value*/
        virtual RET_TYPE& operator <<= (Operable& b) = 0;
        virtual RET_TYPE& operator <<= (ull b) = 0;


        virtual RET_TYPE& operator =   (Operable& b) = 0;
        virtual RET_TYPE& operator =   (ull b) = 0;

        virtual Slice getAssignSlice() = 0;

        /** update event management*/
        std::vector<UpdateEvent*>& getUpdateMeta(){ return _updateMeta; }
        void addUpdateMeta(UpdateEvent* event){
            _updateMeta.push_back(event);
        }

        /** generate update metas*/
        AssignMeta* generateAssignMeta(Operable& assignValue, Slice assignSlice){
            return new AssignMeta(_updateMeta, assignValue, assignSlice);
        }


        /***
         *
         * simulation task
         *
         * */

        /////// assign value to val representation
        ////////////////// usually it is call from sim Interface
        /////// getFromCur means get value from current cycle or from back cycle
        void assignValRepCurCycle(ValRep& desValRep){

            for (auto curUpEvent : _updateMeta){
                assert(curUpEvent != nullptr);
                curUpEvent->trySimAll();
                curUpEvent->tryAssignValRep(desValRep);
            }

        }

        static bool upEventCmp(const UpdateEvent* lhs, const UpdateEvent* rhs){
            assert(lhs != nullptr);
            assert(rhs != nullptr);
            return (*lhs) < (*rhs);
        }

        void sortUpEventByPriority(){
            std::sort(_updateMeta.begin(), _updateMeta.end(), upEventCmp);
        }





    };

    template<typename RET_TYPE>
    class AssignCallbackFromAgent{
    public:
        virtual RET_TYPE& callBackBlockAssignFromAgent(Operable& b, Slice absSliceOfHost)    = 0;
        virtual RET_TYPE& callBackNonBlockAssignFromAgent(Operable& b, Slice absSliceOfHost) = 0;
    };



}

#endif //KATHRYN_ASSIGNABLE_H
