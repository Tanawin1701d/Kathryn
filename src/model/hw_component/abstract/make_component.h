//
// Created by tanawin on 1/12/2566.
//

#ifndef KATHRYN_MAKECOMPONENT_H
#define KATHRYN_MAKECOMPONENT_H

#include "typeinfo"
#include "memory"
#include "type_traits"
#include "model/controller/con_interf/controller_itf.h"
#include "model/hw_component/abstract/identifiable.h"


/** this is for user usage*/
#define m_mod(name, TypeName, ...) TypeName& name = _make<TypeName>(#TypeName, #name, true, __VA_ARGS__)
#define m_wire( name, argument)    Wire&     name = _make<Wire>    ("uncatagorized_yet", #name, true,argument)
#define m_in( name, argument)      Wire&     name = _makeIo<Wire>  (true,"uncatagorized_yet", #name, true,argument, false)
#define m_out( name, argument)     Wire&     name = _makeIo<Wire>  (false,"uncatagorized_yet", #name, true,argument, false)
#define m_reg( name, argument)     Reg&      name = _make<Reg>     ("uncatagorized_yet", #name, true, argument)
#define m_expr(name, argument)     expression& name = _make<expression>("uncatagorized_yet", #name, true, argument)
#define m_val(name, ...)           Val&      name = _make<Val>     ("uncatagorized_yet", #name, true, __VA_ARGS__)
#define m_pm_val(name, default_val)  PmVal&    name = _make<PmVal>   ("uncatagorized_yet", #name, true, default_val)
#define m_mem(name, depth, width)  MemBlock& name = _make<MemBlock>("uncatagorized_yet", #name, true, depth, width)
#define g(...) make_nest(true,__VA_ARGS__)
#define gr(...) make_nest_read_only(true, __VA_ARGS__)
#define g_man(...) make_nest_man(true, __VA_ARGS__)
#define m_box(name, TypeName) TypeName& name = _make<TypeName>("uncatagorized_yet",#name, true)

#define m_opr_reg(vary_name, argument)        _make<Reg>        ("uncatagorized_yet" ,  vary_name, true, argument)
#define m_opr_wire(vary_name, argument)       _make<Wire>       ("uncatagorized_yet" ,  vary_name, true,argument)
#define m_opr_mod(vary_name, TypeName, ...)   _make<TypeName>   (#TypeName, vary_name, true, __VA_ARGS__)
#define m_opr_val(vary_name, ...)             _make<Val>        ("uncatagorized_yet" ,  vary_name, true, __VA_ARGS__)

/** this is for internal use nest is not allow here (only use_nest man)*/
#define make_mod(name, TypeName, ...) Module&   name = _make<TypeName>(#TypeName, #name, false, __VA_ARGS__)
#define make_wire( name, argument)    Wire&     name = _make<Wire>    ("uncatagorized_yet" , #name, false,argument)
#define make_reg( name, argument)     Reg&      name = _make<Reg>     ("uncatagorized_yet" , #name, false, argument)
#define make_val(name, ...)           Val&      name = _make<Val>     ("uncatagorized_yet" , #name, false, __VA_ARGS__)
#define make_mem(name, depth, width)  MemBlock& name = _make<MemBlock>("uncatagorized_yet" , #name, false, depth, width)
#define g_man_internal(...) make_nest_man(false, __VA_ARGS__)
#define g_man_internal_read_only(...) make_nest_man_read_only(false, __VA_ARGS__)

#define make_opr_wire_wo_def( vary_name, argument)  _make<Wire>       ("uncatagorized_yet" ,  vary_name, false,argument, false)
#define make_opr_wire( vary_name, argument)       _make<Wire>       ("uncatagorized_yet" ,  vary_name, false,argument)
#define make_opr_reg( vary_name, argument)        _make<Reg>        ("uncatagorized_yet" ,  vary_name, false, argument)
#define make_opr_val(vary_name, ...)              _make<Val>        ("uncatagorized_yet" ,  vary_name, false, __VA_ARGS__)
#define make_opr_mem(vary_name, depth, width)     _make<MemBlock>   ("uncatagorized_yet" ,  vary_name, false, depth, width)
#define make_opr_proxy_expr(vary_name, size)       _make<expression> ("uncatagorized_yet" ,  vary_name, false, size)
#define make_opr_io_wire(vary_name, size, type)    _make<WireAuto>("uncatagorized_yet" ,  vary_name, true, size, type)
/////#define g(...) make_nest(false,__VA_ARGS__)
#define make_box(name, TypeName) TypeName& name = _make<TypeName>("uncatagorized_yet", #name, false)


/*** box is globally used in userland and internal land*/
#define box(tn) struct tn: Box
#define init_box(tn)  auto& operator=(const tn& rhs) { Box::operator=((Box&) rhs); return *this;};


#define var auto&


namespace kathryn {

    /**
     * This is integrity check bit refer whether it is made from make template
     * it must set from made and
     **/

    void unlock_alloc();

    template<typename T, typename... Args>
    T& _make(const std::string& type_name, const std::string& name, bool is_user_dec,Args&&... args){
        static_assert(std::is_base_of<HwCompControllerItf, T>::value,
                "make model component must base on ModelController controllable"
                );
        static_assert(std::is_base_of<IdentBase, T>::value,
                      "make model component must base on Identifiable"
                );

        /** make initializer*/
        unlock_alloc();
        set_retrieve_var_meta(type_name, name, is_user_dec);
        auto obj_ptr = new T(std::forward<Args>(args)...);
        obj_ptr->com_final(); /** /* typicallly it is used only module and box*/

        return *obj_ptr;
    }

    template<typename T, typename... Args>
    T& _makeIo(bool is_input, const std::string& type_name, const std::string& name, bool is_user_dec,Args&&... args){
        T& x = _make<T>(type_name, name, is_user_dec, args...);
        if (is_input){
            x.as_input();
        }else{
            x.as_output();
        }
        return x;
    }

    // we will use it later for declaration of zero argument
    // template<typename T>
    // T& _make(const std::string name, bool is_user_dec){
    //     static_assert(std::is_base_of<HwCompControllerItf, T>::value,
    //                   "make model component must base on ModelController controllable"
    //     );
    //     static_assert(std::is_base_of<IdentBase, T>::value,
    //                   "make model component must base on Identifiable"
    //     );
    //
    //     /** make initializer*/
    //     unlock_alloc();
    //     set_retrieve_var_meta(name, is_user_dec);
    //     auto obj_ptr = new T();
    //     obj_ptr->com_final(); /** /* typicallly it is used only module and box*/
    //
    //     return *obj_ptr;
    // }

}

#endif //KATHRYN_MAKECOMPONENT_H
