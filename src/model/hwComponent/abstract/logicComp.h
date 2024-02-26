//
// Created by tanawin on 10/12/2566.
//

#ifndef KATHRYN_LOGICCOMP_H
#define KATHRYN_LOGICCOMP_H


#include "model/hwComponent/abstract/slicable.h"
#include "model/hwComponent/abstract/operation.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/identifiable.h"
#include "model/controller/conInterf/controllerItf.h"
#include "model/simIntf/modelSimEngine.h"
#include "model/simIntf/logicSimInterface.h"
#include "model/debugger/modelDebugger.h"


namespace kathryn{

    template<typename TYPE_COMP>
    class LogicComp : public AssignOpr<TYPE_COMP>,
                      public Assignable,
                      public Operable,
                      public Slicable<TYPE_COMP>,
                      public AssignCallbackFromAgent<TYPE_COMP>,
                      public Identifiable,
                      public HwCompControllerItf,
                      public ModelDebuggable,
                      public SimEngineInterface{
    protected:
        SimEngine* _simEngine =  nullptr;

    public:

        explicit LogicComp(Slice slc,
                           HW_COMPONENT_TYPE hwType,
                           SimEngine* simEngine,
                           bool requiredAllocCheck):
                AssignOpr<TYPE_COMP>(),
                Assignable(),
                Operable(),
                Slicable<TYPE_COMP>(slc),
                Identifiable(hwType),
                HwCompControllerItf(requiredAllocCheck),
                ////LogicSimEngine(slc.getSize(), sigType, simForNext),
                ModelDebuggable(),
                SimEngineInterface(),
                _simEngine(simEngine)
                            {}

        virtual ~LogicComp() {
            delete _simEngine;
        };

        SimEngine* getSimEngine() override{
            return _simEngine;
        }


        /** iterable override*/

        std::string getMdIdentVal() override{
            return getIdentDebugValue();
        }

        Operable* castToOpr(){
            return static_cast<Operable*>(this);
        }

        /** operable override*/
        Slice getOperableSlice() const override{
            return Slicable<TYPE_COMP>::getSlice();
        }

        Operable& getExactOperable() const override{
            return *(Operable*)this;
        }

//        Operable* doSlice(Slice sl) override{
//            auto x = Slicable<TYPE_COMP>::operator() (sl.start, sl.stop);
//            return x.castToOperable();
//        }

        Simulatable* getSimItf() override{
            return static_cast<Simulatable*>(getSimEngine());
        }

        RtlValItf* getRtlValItf() override{
            return static_cast<RtlValItf*>(getSimEngine());
        }

        Identifiable* castToIdent() override{
            return static_cast<Identifiable*>(this);
        }
        ValRep& sv() override{
            _simEngine->setCurValSimStatus();
            return _simEngine->getCurVal();
        }

        /**
         *
         * override assignable
         *
         * */
        Slice getAssignSlice() override{
            return Slicable<TYPE_COMP>::getSlice();
        }

        void startCheckShortCircuit(){
            Operable* matchOpr = checkShortCircuit();
            if (matchOpr != nullptr){
                std::cout << getMdIdentVal() << "  match with " << matchOpr->castToIdent()->getIdentDebugValue() << std::endl;
                mfAssert(false, "get short circuit");
            }
        }

    };

}

#endif //KATHRYN_LOGICCOMP_H
