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
#include "model/simIntf/modelSimInterface.h"
#include "model/debugger/modelDebugger.h"

namespace kathryn{

    template<typename T>
    class LogicComp : public Assignable<T>, public Operable,
                      public Slicable<T>,
                      public AssignCallbackFromAgent<T>,
                      public Identifiable,
                      public HwCompControllerItf,
                      public RtlSimulatable,
                      public ModelDebuggable{
    public:
        explicit LogicComp(Slice slc, HW_COMPONENT_TYPE hwType,
                           RtlSimEngine* simEngine, bool requiredAllocCheck):
                Assignable<T>(),
                Operable(),
                Slicable<T>(slc),
                Identifiable(hwType),
                HwCompControllerItf(requiredAllocCheck),
                RtlSimulatable(simEngine),
                ModelDebuggable()
                            {}

        virtual ~LogicComp() = default;

        Identifiable * castToIdent() override{
            return static_cast<Identifiable*>(this);
        }

        std::string getMdIdentVal() override{
            return getIdentDebugValue();
        }

        RtlSimulatable* castToRtlSimItf() override{
            return static_cast<RtlSimulatable*>(this);
        };

        ValRep& getExactSimCurValue() override{
            return getSimEngine()->getCurVal();
        }

        ValRep& getExactSimNextValue() override{
            return getSimEngine()->getNextVal();
        }

        ValRep& sv() override{
            getSimEngine()->setCurValSimStatus();
            return getSimEngine()->getCurVal();
        }

    };

}

#endif //KATHRYN_LOGICCOMP_H
