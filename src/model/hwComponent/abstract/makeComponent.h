//
// Created by tanawin on 1/12/2566.
//

#ifndef KATHRYN_MAKECOMPONENT_H
#define KATHRYN_MAKECOMPONENT_H

#include <typeinfo>
#include <memory>
#include <type_traits>
#include "model/controller/controller.h"
#include "model/controller/conInterf/controllerItf.h"
#include "model/hwComponent/abstract/identifiable.h"


#define makeWire( name, argument) Wire& name = _make<Wire>(argument)
#define makeReg( name, argument)  Reg& name = _make<Reg>(argument)
#define makeMod(name, TypeName, ...) Module& name = _make<TypeName>(__VA_ARGS__)
#define var auto&


namespace kathryn {

    /**
     * This is integrity check bit refer whether it is made from make template
     * it must set from made and
     **/

    template<typename T, typename... Args>
    T& _make(Args&&... args){
        static_assert(std::is_base_of<HwCompControllerItf, T>::value,
                "make model component must base on Controller controllable"
                );
        static_assert(std::is_base_of<Identifiable, T>::value,
                      "make model component must base on Identifiable"
                );

        /** make initializer*/
        getControllerPtr()->unlockAllocation();
        auto objPtr = new T(std::forward<Args>(args)...);
        objPtr->com_final();
        objPtr->setTypeName(typeid(T).name());
        return *objPtr;
    }

}

#endif //KATHRYN_MAKECOMPONENT_H
