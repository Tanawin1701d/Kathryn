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


/** this is for user usage*/



/** this is for internal use*/
#define makeMod(name, TypeName, ...) Module&   name = _make<TypeName>(#name, false, __VA_ARGS__)
#define makeWire( name, argument)    Wire&     name = _make<Wire>    (#name, false,argument)
#define makeReg( name, argument)     Reg&      name = _make<Reg>     (#name, false, argument)
#define makeVal(name, ...)           Val&      name = _make<Val>     (#name, false, __VA_ARGS__)
#define makeMem(name, depth, width)  MemBlock& name = _make<MemBlock>(#name, false, depth, width)
#define g(...) makeNest(false,__VA_ARGS__)
#define box(tn) struct tn: Box
#define initBox(tn)  auto& operator=(const tn& rhs) { Box::operator=((Box&) rhs); return *this;};
#define makeBox(name, TypeName) TypeName& name = _make<TypeName>(#name, false)


#define var auto&


namespace kathryn {

    /**
     * This is integrity check bit refer whether it is made from make template
     * it must set from made and
     **/

    void unlockAlloc();



    template<typename T, typename... Args>
    T& _make(const std::string name, bool isUserDec,Args&&... args){
        static_assert(std::is_base_of<HwCompControllerItf, T>::value,
                "make model component must base on ModelController controllable"
                );
        static_assert(std::is_base_of<IdentBase, T>::value,
                      "make model component must base on Identifiable"
                );

        /** make initializer*/
        unlockAlloc();
        setRetrieveVarMeta(name, isUserDec);
        auto objPtr = new T(std::forward<Args>(args)...);
        objPtr->com_final(); /** /* typicallly it is used only module and box*/

        return *objPtr;
    }

    template<typename T>
    T& _make(const std::string name, bool isUserDec){
        static_assert(std::is_base_of<HwCompControllerItf, T>::value,
                      "make model component must base on ModelController controllable"
        );
        static_assert(std::is_base_of<IdentBase, T>::value,
                      "make model component must base on Identifiable"
        );

        /** make initializer*/
        unlockAlloc();
        setRetrieveVarMeta(name, isUserDec);
        auto objPtr = new T();
        objPtr->com_final(); /** /* typicallly it is used only module and box*/

        return *objPtr;
    }

}

#endif //KATHRYN_MAKECOMPONENT_H
