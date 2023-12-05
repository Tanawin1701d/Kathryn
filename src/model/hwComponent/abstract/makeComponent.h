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



namespace kathryn {

    /**
     * This is integrity check bit refer whether it is made from make template
     * it must set from made and
     **/


    /** This is used for represent */
    void* declaredAddr;

    /** todo for now it is used for creating module but we will make it compatable in the future */
    template<typename T, typename... Args>
    T& make(Args&&... args){
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
