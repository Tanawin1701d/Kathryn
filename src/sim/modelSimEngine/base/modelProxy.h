//
// Created by tanawin on 31/5/2024.
//

#ifndef MODELPROXY_H
#define MODELPROXY_H

#include <cassert>
#include <utility>


#include "model/hwComponent/abstract/operable.h"
#include "proxyEventBase.h"
#include "sim/simResWriter/simResWriter.h"
#include "model/abstract/identBase/identBase.h"
#include "sim/logicRep/valRep.h"
#include "util/fileWriter/codeWriter/cppWriter.h"
#include "simValType.h"


namespace kathryn{

    constexpr int MAX_VAL_REP_SIZE = 64;


    class ModelProxyBuild{
    protected:
        std::vector<ModelProxyBuild*> dep;
        std::string TEMP_VAR_SUFFIX = "_TEMP";
    public:
        virtual             ~ModelProxyBuild()= default;
        virtual void        proxyBuildInit()   = 0;
        //// pre initialize section fill dependency
        virtual std::string              getVarName()      = 0; //// prefix is not include
        virtual std::vector<std::string> getRegisVarName() = 0;
        virtual ull                      getVarId()        = 0; //// it is global id
        std::vector<ModelProxyBuild*>&   getDep() {return dep;}
        virtual SIM_VALREP_TYPE          getValR_Type()    = 0;

        //// c++ create section
        virtual void createGlobalVariable (CbBaseCxx& cb) = 0;
        virtual void createLocalVariable  (CbBaseCxx& cb) = 0;
        virtual void createOp             (CbBaseCxx& cb) = 0; //// compute the data but
        virtual void createOpEndCycle     (CbBaseCxx& cb) = 0; //// have more priority
        virtual void createOpEndCycle2    (CbBaseCxx& cb) = 0; //// have less priority
        virtual bool isUserDeclare()        = 0;


    };

    class ModelProxyRetrieve{
    protected:
        ValRepBase proxyRep;
    public:
        virtual void        proxyRetInit(ProxySimEventBase* modelSimEvent) = 0;
        virtual ValRepBase& getProxyRep () = 0;
        virtual      ~ModelProxyRetrieve() = default;
    };

    

}

#endif //MODELPROXY_H
