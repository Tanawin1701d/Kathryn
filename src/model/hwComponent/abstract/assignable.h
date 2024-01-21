//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_ASSIGNABLE_H
#define KATHRYN_ASSIGNABLE_H

#include <vector>

#include "operable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/Slice.h"
#include "model/simIntf/rtlSimEle.h"

namespace kathryn{

    /** reg/wire update metas data*/
    struct UpdateEvent{
        Operable* updateCondition = nullptr; /// which condition that allow this value to update.
        Operable* updateState     = nullptr; /// which state that need to update.
        Operable* updateValue     = nullptr; /// value to update.
        Slice     updateSlice; /// slice to update
        int priority = 9;
        ///priority for circuit if there are attention to update same register at a time 0 is highest 9 is lowest

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
                if (samplingVal != 1){
                    return false;
                }
            }
            return true;
        }

        void tryAssignValRep(ValRep& desValRep, bool getFromCur){
            ////// check update state valid
            if (!(shouldAssignValRep(updateCondition, getFromCur) ||
                  shouldAssignValRep(updateState, getFromCur))){
                return;
            }
            assert(updateValue != nullptr);

            ValRep& vr = getFromCur ? updateValue->castToRtlSimItf()->getSimEngine()->getCurVal()
                                    : updateValue->castToRtlSimItf()->getSimEngine()->getBackVal();
                    ;
            desValRep.updateOnSlice(vr, updateSlice);
        }

        void trySimAll(){
            trySim(updateCondition);
            trySim(updateState);
            trySim(updateValue);
        }

        static void trySim(Operable* opr){
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
        virtual ~Assignable(){
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

        /////// assign value to val representation
        ////////////////// usually it is call from sim Interface
        /////// getFromCur means get value from current cycle or from back cycle
        void assignValRepCurCycle(ValRep& desValRep, bool getFromCur){

            for (int upId = _updateMeta.size()-1; upId >= 0; upId--){
                UpdateEvent* curUpEvent = _updateMeta[upId];
                curUpEvent->trySimAll();
                assert(curUpEvent != nullptr);
                curUpEvent->tryAssignValRep(desValRep, getFromCur);
            }


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
