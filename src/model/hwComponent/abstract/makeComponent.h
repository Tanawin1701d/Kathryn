//
// Created by tanawin on 1/12/2566.
//

#ifndef KATHRYN_MAKECOMPONENT_H
#define KATHRYN_MAKECOMPONENT_H

#include <typeinfo>
#include <memory>
#include <type_traits>
#include "model/controller/conInterf/controllerItf.h"
#include "model/hwComponent/abstract/identifiable.h"


#define makeWire( name, argument) Wire& name = _make<Wire>(#name,argument)
#define makeReg( name, argument)  Reg& name = _make<Reg>(#name, argument)
#define makeVal(name, ...) Val& name = _make<Val>(#name, __VA_ARGS__)
#define makeMem(name, depth, width) MemBlock& name = _make<MemBlock>(#name, depth, width)
#define makeMod(name, TypeName, ...) Module& name = _make<TypeName>(#name, __VA_ARGS__)
#define var auto&


namespace kathryn {

    /**
     * This is integrity check bit refer whether it is made from make template
     * it must set from made and
     **/

    void unlockAlloc();

    template<typename T, typename... Args>
    T& _make(const std::string name,Args&&... args){
        static_assert(std::is_base_of<HwCompControllerItf, T>::value,
                "make model component must base on ModelController controllable"
                );
        static_assert(std::is_base_of<Identifiable, T>::value,
                      "make model component must base on Identifiable"
                );

        /** make initializer*/
        unlockAlloc();
        auto objPtr = new T(std::forward<Args>(args)...);
        objPtr->setVarName(name);
        objPtr->com_final();

        return *objPtr;
    }

}

#endif //KATHRYN_MAKECOMPONENT_H
