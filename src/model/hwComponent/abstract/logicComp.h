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

    template<typename T>
    class LogicComp : public Assignable<T>, public Operable,
                      public Slicable<T>,
                      public AssignCallbackFromAgent<T>,
                      public Identifiable,
                      public HwCompControllerItf,
                      public LogicSimulatable,
                      public ModelDebuggable{
    public:
        explicit LogicComp(Slice slc,
                           HW_COMPONENT_TYPE hwType,
                           VCD_SIG_TYPE sigType,
                           bool simForNext,
                           bool requiredAllocCheck):
                Assignable<T>(),
                Operable(),
                Slicable<T>(slc),
                Identifiable(hwType),
                HwCompControllerItf(requiredAllocCheck),
                LogicSimulatable(slc.getSize(), sigType, simForNext),
                ModelDebuggable()
                            {}

        virtual ~LogicComp() = default;


        /** iterable override*/

        std::string getMdIdentVal() override{
            return getIdentDebugValue();
        }

        Operable* castToOpr(){
            return static_cast<Operable*>(this);
        }

        /** operable override*/
        Slice getOperableSlice() const override{
            return Slicable<T>::getSlice();
        }
        Operable& getExactOperable() const override{
            return *(Operable*)this;
        }
        Simulatable* getSimItf() override{
            return static_cast<Simulatable*>(this);
        }
        RtlValItf* getRtlValItf() override{
            return static_cast<RtlValItf*>(this);
        }
        Identifiable * castToIdent() override{
            return static_cast<Identifiable*>(this);
        }
        ValRep& sv() override{
            setCurValSimStatus();
            return getCurVal();
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
