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
#include "model/simIntf/rtlSimEle.h"
#include "model/simIntf/rtlSimItf.h"
#include "model/debugger/modelDebugger.h"

namespace kathryn{

    template<typename T>
    class LogicComp : public Assignable<T>, public Operable,
                      public Slicable<T>,
                      public AssignCallbackFromAgent<T>,
                      public Identifiable,
                      public HwCompControllerItf,
                      public SimInterface,
                      public ModelDebuggable{
    public:
        explicit LogicComp(Slice slc, HW_COMPONENT_TYPE hwType,
                           bool requiredAllocCheck):
                            Assignable<T>(),
                            Operable(),
                            Slicable<T>(slc),
                            Identifiable(hwType),
                            HwCompControllerItf(requiredAllocCheck),
                            SimInterface(new RtlSimEngine(slc.getSize())),
                            ModelDebuggable()
                            {}

        virtual ~LogicComp() = default;

        Identifiable * castToIdent() override{
            return static_cast<Identifiable*>(this);
        }

        std::string getMdIdentVal() override{
            return getIdentDebugValue();
        }

        SimInterface* castToSimItf() override{
            return static_cast<SimInterface*>(this);
        };

    };

}

#endif //KATHRYN_LOGICCOMP_H
