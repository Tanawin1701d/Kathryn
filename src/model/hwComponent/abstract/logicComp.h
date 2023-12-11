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
#include "model/hwComponent/abstract/hwDebuggable.h"
#include "model/controller/conInterf/controllerItf.h"

namespace kathryn{

    template<typename T>
    class LogicComp : public Assignable<T>, public Operable,
                      public Slicable<T>,
                      public AssignCallbackFromAgent<T>,
                      public Identifiable,
                      public HwCompControllerItf,
                      public HwCompDebuggable{
    public:
        explicit LogicComp(Slice slc, HW_COMPONENT_TYPE hwType, bool requiredAllocCheck):
                            Assignable<T>(),
                            Operable(),
                            Slicable<T>(slc),
                            Identifiable(hwType),
                            HwCompControllerItf(requiredAllocCheck),
                            HwCompDebuggable()
                            {}

        virtual ~LogicComp() = default;

        Identifiable * castToIdent() override{
            return static_cast<Identifiable*>(this);
        }

    };

}

#endif //KATHRYN_LOGICCOMP_H
