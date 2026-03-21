//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_IDENTIFIABLE_H
#define KATHRYN_IDENTIFIABLE_H

#include "string"
#include "utility"
#include "memory"
#include "cassert"
#include "vector"
#include "model/debugger/model_debugger.h"

#include "model/abstract/ident_base/ident_base.h"


typedef unsigned long long int ull;

namespace kathryn {

    struct VarMeta{
        std::string var_type; //// for module use module type
        std::string var_name;
        bool        is_user;

        bool operator == (const VarMeta& rhs) const {
            return (var_type == rhs.var_type) &
                   (var_name == rhs.var_name) &
                   (is_user  == rhs.is_user );
        } 
    };

    bool    is_var_name_retrievable(ull device_ident_id);
    VarMeta retrieve_var_meta();
    void    set_retrieve_var_meta(std::string var_type,
                               std::string name,
                               bool is_user_dec);


    enum HW_COMPONENT_TYPE{
        TYPE_REG = 0,
        TYPE_STATE_REG = 1,
        TYPE_COND_WAIT_STATE_REG = 2,
        TYPE_CYCLE_WAIT_STATE_REG = 3,
        TYPE_WIRE = 4,
        TYPE_EXPRESSION = 5,
        TYPE_NEST = 6,
        TYPE_MODULE = 7,
        TYPE_VAL = 8,
        TYPE_MEM_BLOCK = 9,
        TYPE_MEM_BLOCK_INDEXER = 10,
        TYPE_BOX = 11,
        TYPE_INTF = 12,
        TYPE_PMVAL = 13,
        TYPE_COUNTER_REG = 14,
        TYPE_COUNT = 15
    };

    static const std::string GLOBAL_PREFIX[TYPE_COUNT] = {"REG",
                                                          "SR_ST",
                                                          "SR_CDWT",
                                                          "SR_CYWT",
                                                          "WIRE",
                                                          "EXPR",
                                                          "NEST",
                                                          "MODULE",
                                                          "VAL",
                                                          "MEM_BLOCK",
                                                          "MEM_BLOCK_INDEXER",
                                                          "BOX",
                                                          "ITF",
                                                          "PMVAL",
                                                          "CNT_REG"
                                                        };

    class Module;
    class Identifiable: public IdentBase {
    protected:
        const std::string UNNAME_STR = "unnamed";
        /** name type such as Reg Wire ModuleClassName*/
        HW_COMPONENT_TYPE _type;
        VarMeta           _varMeta; /// sub type of component typically we use for module
        /** local variable*/
        Module* _parent; /// if it is nullptr it is not localized
        /// it will share among the same module

    public:
        /** assign and auto increment object id */
        explicit Identifiable(HW_COMPONENT_TYPE type) :
                IdentBase(),
                _type(type),
                _varMeta({"UN_INIT_TYPE","UN_INIT_VN", false}),
                _parent(nullptr)
            {

            if (is_var_name_retrievable(_globalId)){ _varMeta = retrieve_var_meta();}
            if (!_varMeta.is_user){ _varMeta.var_name += "_SYS";}
            _globalName = GLOBAL_PREFIX[type] + std::to_string(_globalId);

            if (_globalId ==78){
                //mf_assert(false, "dddddd");
                int x = 0;
                if (_globalId == 5088){
                    x = 1;
                }
            }

            };

        virtual ~Identifiable() = default;

        Identifiable& operator = (const Identifiable& ident){
            if (this == &ident){
                return *this;
            }
            _type       = ident._type;
            _varMeta    = ident._varMeta;
            _varMeta.var_name += "_CP";
            _parent     = ident._parent;
            return *this;
        }

        /** get hardware component type*/
        [[nodiscard]]
        HW_COMPONENT_TYPE  get_type() const {return _type;}
        /** get/set type_name (variable name)*/
        [[nodiscard]]
        const std::string& get_var_name() const {return _varMeta.var_name;}
        void               set_var_name(std::string type_name) { _varMeta.var_name = std::move(type_name);}
        bool               is_user_var() const {return _varMeta.is_user;}
        [[nodiscard]]
        VarMeta&           get_var_meta() {return _varMeta;}

        /**build Inherit varname*/
        void               build_inherit_name() override;

        Module*get_parent_ptr(){return _parent;}
        void               set_parent(Module* parent) {_parent = parent;}

        /** get debug value*/
        [[nodiscard]]std::string
        get_ident_debug_value() const{
            return get_global_name() + "_localName_" + get_var_name();
        }

    };

}

#endif //KATHRYN_IDENTIFIABLE_H
