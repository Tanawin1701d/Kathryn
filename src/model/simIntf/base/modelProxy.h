//
// Created by tanawin on 31/5/2024.
//

#ifndef MODELPROXY_H
#define MODELPROXY_H

#include <cassert>
#include <utility>
#include "proxyEventBase.h"

#include "sim/simResWriter/simResWriter.h"

#include "model/abstract/identBase/identBase.h"
#include "sim/logicRep/valRep.h"

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
        virtual std::string  getVarName()       = 0; //// prefix is not include
        virtual ull          getVarId()         = 0; //// it is global id
        std::vector<ModelProxyBuild*>& getDep() {return dep;}
        std::string          sliceVar(std::string varName, Slice sl){
            return varName + ".slice<" + std::to_string(sl.start) +
                   ", " + std::to_string(sl.stop) + ">()";
        }

        //// c++ create section
        virtual std::string createVariable()      = 0;
        virtual std::string createOp()            = 0; //// at the middle of the cycle
        virtual std::string createOpEndCycle()    = 0; //// at the end of cycle
        virtual bool        isUserDeclare()       = 0;


    };

    class ModelProxyRetrieve{
    protected:
        ValRepBase* proxyRep  = nullptr;
    public:
        virtual void        proxyRetInit(ProxySimEventBase* modelSimEvent) = 0;
        virtual ValRepBase* getProxyRep () = 0;
        virtual      ~ModelProxyRetrieve() = default;
    };

    

}

#endif //MODELPROXY_H
