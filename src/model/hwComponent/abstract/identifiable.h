//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_IDENTIFIABLE_H
#define KATHRYN_IDENTIFIABLE_H

#include <string>
#include <utility>
#include <memory>
#include <cassert>
#include <vector>
#include "model/debugger/modelDebugger.h"

#include "model/abstract/identBase/identBase.h"


typedef unsigned long long int ull;

namespace kathryn {

    struct VarMeta{
        std::string varName;
        bool        isUser;
    };

    bool    isVarNameRetrievable(ull deviceIdentId);
    VarMeta retrieveVarMeta();
    void    setRetrieveVarMeta(std::string name, bool isUserDec);


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
        TYPE_COUNT = 12
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
                                                          "BOX"
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
                _varMeta({"UN_INIT", false}),
                _parent(nullptr)
            {

            if (isVarNameRetrievable(_globalId)){ _varMeta = retrieveVarMeta();}
            if (!_varMeta.isUser){ _varMeta.varName += "INTERNAL";}
            _globalName = GLOBAL_PREFIX[type] + std::to_string(_globalId);

            if (_globalId == 19){
                //mfAssert(false, "dddddd");
                int x = 0;
            }

            };

        virtual ~Identifiable() = default;

        Identifiable& operator = (const Identifiable& ident){
            if (this == &ident){
                return *this;
            }
            _type       = ident._type;
            _varMeta    = ident._varMeta;
            _varMeta.varName += "_CP";
            _parent     = ident._parent;
            return *this;
        }

        /** get hardware component type*/
        [[nodiscard]]
        HW_COMPONENT_TYPE  getType() const {return _type;}
        /** get/set typeName (variable name)*/
        [[nodiscard]]
        const std::string& getVarName() const {return _varMeta.varName;}
        void               setVarName(std::string typeName) { _varMeta.varName = std::move(typeName);}

        bool               isUserVar() const {return _varMeta.isUser;}

        /**build Inherit varname*/
        void               buildInheritName() override;

        Module*            getParent(){return _parent;}
        void               setParent(Module* parent) {_parent = parent;}

        /** get debug value*/
        [[nodiscard]]std::string
        getIdentDebugValue() const{
            return getGlobalName() + "_localName_" + getVarName();
        }

    };

}

#endif //KATHRYN_IDENTIFIABLE_H
