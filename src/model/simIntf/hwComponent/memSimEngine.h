//
// Created by tanawin on 31/5/2024.
//

#ifndef MODEL_SIMINTF_MEMSIMENGINE_H
#define MODEL_SIMINTF_MEMSIMENGINE_H


#include "model/simIntf/base/modelProxy.h"

namespace kathryn{

    class MemBlock;

    class MemSimEngine: public ModelProxyBuild, public ModelProxyRetrieve{
    private:
        MemBlock* _master = nullptr;
    public:
        explicit MemSimEngine(MemBlock* master);

        ///////// create c++
        void        proxyBuildInit()       override{};
        std::string getVarName()           override;
        ull         getVarId()             override;
        
        std::string createVariable()       override;
        std::string createOp()             override{return "";}
        std::string createOpEndCycle()     override{return "";}

        bool        isUserDeclare()        override{return false;}

        void        proxyRetInit(ProxySimEventBase* modelSimEvent)override;
        ValRepBase* getProxyRep()          override;

    };

    class MemSimEngineInterface{
    public:
        virtual ~MemSimEngineInterface() = default;
        virtual MemSimEngine* getSimEngine() = 0;
    };



}

#endif //MODEL_SIMINTF_MEMSIMENGINE_H
