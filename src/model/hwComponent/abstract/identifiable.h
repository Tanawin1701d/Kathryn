//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_IDENTIFIABLE_H
#define KATHRYN_IDENTIFIABLE_H

#include <string>
#include <utility>
#include <memory>
#include <cassert>

typedef unsigned long long int ull;

namespace kathryn {

    extern ull LAST_IDENT_ID;


    enum HW_COMPONENT_TYPE{
        TYPE_REG = 0,
        TYPE_STATE_REG = 1,
        TYPE_COND_WAIT_STATE_REG = 2,
        TYPE_CYCLE_WAIT_STATE_REG = 3,
        TYPE_WIRE = 4,
        TYPE_EXPRESSION = 5,
        TYPE_MODULE = 6,
        TYPE_VAL = 7,
        TYPE_COUNT = 8
    };

    static const std::string GLOBAL_PREFIX[TYPE_COUNT] = {"R",
                                                          "S",
                                                          "W",
                                                          "E",
                                                          "M",
                                                          "V"
                                                        };

    class Module;
    class Identifiable {
    private:
        const std::string UNNAME_STR = "unnamed";
        /** name type such as Reg Wire ModuleClassName*/
        HW_COMPONENT_TYPE _type;
        std::string _typeName; /// sub type of component typically we use for module
        std::string _globalName;
        ull         _globalId; /// id that DISTINCT // for all element even it is the same type
        /// name for debugging

        /** local variable*/
        Module* _parent; /// if it is nullptr it is not localized
        ull     _localId; /// id that use in the component
        /// it will share among the same module

    public:
        /** assign and auto increment object id */
        explicit Identifiable(HW_COMPONENT_TYPE type) :
            _type(type),
            _typeName(UNNAME_STR),
            //_globalName(UNNAME_STR),
            _globalId(LAST_IDENT_ID),
            _parent(nullptr),
            _localId(-1)
            {
            _globalName = GLOBAL_PREFIX[type] + std::to_string(LAST_IDENT_ID);
            LAST_IDENT_ID++;

            };

        Identifiable& operator = (const Identifiable& ident){
            if (this == &ident){
                return *this;
            }
            _type = ident._type;
            _typeName = UNNAME_STR;
            _globalName = UNNAME_STR;
            _globalId = LAST_IDENT_ID++;
            _parent = nullptr;
            _localId = -1;
            return *this;
        }

        bool isLocalized() {return _parent != nullptr;}

        /** get hardware component type*/
        [[nodiscard]] HW_COMPONENT_TYPE getType() const {return _type;}
        void setIdentType(HW_COMPONENT_TYPE identType){_type = identType;}
        /** get/set typeName (variable name)*/
        [[nodiscard]] const std::string& getTypeName() const {return _typeName;}
        void setTypeName(std::string typeName) {_typeName = std::move(typeName);}
        /** set global name*/
        [[nodiscard]] const std::string& getGlobalName() const {return _globalName;}
        void setGlobalName(const std::string& globalName) {_globalName = globalName;}
        [[nodiscard]] ull getGlobalId() const {return _globalId;}
        /// global id can not be set it permanent when class is initialized void setGlobalId(ull globalId) {_globalId = globalId;}

        Module* getParent(){return _parent;}
        void setParent(Module* parent) {_parent = parent;}

        [[nodiscard]] ull  getLocalId() const{return _localId;}
        void setLocalId(ull id)      {_localId = id; }

        virtual bool castDownAble(){return true;}

        /** get debug value*/
        [[nodiscard]]std::string
        getIdentDebugValue() const{
            return getGlobalName() + "_localName_" + getTypeName();
        }

    };

}

#endif //KATHRYN_IDENTIFIABLE_H
