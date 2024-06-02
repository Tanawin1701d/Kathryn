//
// Created by tanawin on 31/5/2024.
//

#ifndef MODELPROXY_H
#define MODELPROXY_H

#include <cassert>
#include <utility>
#include "model/abstract/identBase/identBase.h"
#include "sim/logicRep/valRep.h"

namespace kathryn{

    constexpr std::string TEMP_VAR_SUFFIX = "_TEMP";

    constexpr int MAX_VAL_REP_SIZE = 64;


    class ModelProxyBuild{
    protected:
        std::vector<ModelProxyBuild*> dep;

    public:
        virtual             ~ModelProxyBuild() = 0;
        virtual void        proxyBuildInit()   = 0;
        //// pre initialize section fill dependency
        virtual std::string getVarName()       = 0; //// prefix is not include
        virtual ull         getVarId()         = 0; //// it is global id
        std::vector<ModelProxyBuild*>& getDep() {return dep;}

        //// c++ create section
        virtual std::string createVariable()      = 0;
        virtual std::string createOp()            = 0;
        virtual std::string createMemorizeOp()    = 0;
        virtual std::string createMemBlkAssOp()   = 0;

        virtual bool        isUserDeclare()       = 0;
        virtual bool        isFlowCollectMeta();


    };

    class ModelProxyRetrieve{
    protected:
        ull _proxyId = -1;
    public:
        virtual void proxyRetInit()                 = 0;
        virtual      ~ModelProxyRetrieve()          = 0;
        virtual void proxyRetrInit(ull proxyId){_proxyId = proxyId;}
    };

    

}

#endif //MODELPROXY_H
