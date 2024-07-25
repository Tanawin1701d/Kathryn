//
// Created by tanawin on 31/5/2024.
//

#ifndef MODEL_SIMINTF_MEMSIMENGINE_H
#define MODEL_SIMINTF_MEMSIMENGINE_H


#include "sim/modelSimEngine/base/modelProxy.h"
#include "sim/modelSimEngine/base/proxyEventBase.h"


namespace kathryn{

    class MemBlock;

    class MemSimEngine: public ModelProxyBuild, public ModelProxyRetrieve{
    private:
        MemBlock* _master = nullptr;
    public:
        explicit MemSimEngine(MemBlock* master);

        ///////// create c++
        void                     proxyBuildInit()  override{};
        ValR                     getValRep()      override;
        std::vector<std::string> getRegisVarName() override;
        ull                      getVarId()        override;
        SIM_VALREP_TYPE          getValR_Type()    override;
        
        void createGlobalVariable(CbBaseCxx& cb) override;
        void createLocalVariable (CbBaseCxx& cb) override{}
        void createOp            (CbBaseCxx& cb) override{}
        void createOpEndCycle    (CbBaseCxx& cb) override{}
        void createOpEndCycle2   (CbBaseCxx& cb) override{}

        bool        isUserDeclare()        override{return false;}
        void        proxyRetInit(ProxySimEventBase* modelSimEvent)override;
        ValRepBase& getProxyRep()          override;

    };

    class MemSimEngineInterface{
    public:
        virtual ~MemSimEngineInterface() = default;
        virtual MemSimEngine* getSimEngine() = 0;
    };



}

#endif //MODEL_SIMINTF_MEMSIMENGINE_H
