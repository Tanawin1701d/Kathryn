//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_IDENTIFIABLE_H
#define KATHRYN_IDENTIFIABLE_H

#include <string>
#include <utility>
#include <memory>

typedef unsigned long long int ull;

namespace kathryn {

    extern ull LAST_IDENT_ID;


    enum HW_COMPONENT_TYPE{
        TYPE_REG,
        TYPE_WIRE,
        TYPE_EXPRESSION,
        TYPE_MODULE,
        TYPE_VAL

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
        std::shared_ptr<Module> _parent; /// if it is nullptr it is not localized
        ull                     _localId; /// id that use in the component
        /// it will share among the same module

    public:
        /** assign and auto increment object id */
        explicit Identifiable(HW_COMPONENT_TYPE type) :
            _type(type),
            _typeName(UNNAME_STR),
            _globalName(UNNAME_STR),
            _globalId(LAST_IDENT_ID++),
            _parent(nullptr),
            _localId(-1)
            {};

        template<typename T>
        std::shared_ptr<T> cast(){
            return std::make_shared<T>(this);
        }

        bool isLocalized() {return _parent != nullptr;}

        /** get set method*/
        HW_COMPONENT_TYPE getType() const {return _type;}

        const std::string& getTypeName() const {return _typeName;}
        void setTypeName(std::string typeName) {_typeName = std::move(typeName);}

        const std::string& getGlobalName() const {return _globalName;}
        void setGlobalName(const std::string& globalName) {_globalName = globalName;}

        ull getGlobalId() const {return _globalId;}
        /// global id can not be set it permanent when class is initialized void setGlobalId(ull globalId) {_globalId = globalId;}

        std::shared_ptr<Module> getParent(){return _parent;}
        void setParent(std::shared_ptr<Module> parent) {_parent = std::move(parent);}

        ull  getLocalId(ull id) const{return _localId;}
        void setLocalId(ull id)      {_localId = id; }



    };

    typedef std::shared_ptr<Identifiable> IdentifiablePtr;

}

#endif //KATHRYN_IDENTIFIABLE_H
