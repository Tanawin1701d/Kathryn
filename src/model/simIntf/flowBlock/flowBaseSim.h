//
// Created by tanawin on 29/3/2567.
//



#ifndef KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H
#define KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H

#include <model/simIntf/base/modelProxy.h>

#include "sim/simResWriter/simResWriter.h"

#include "model/simIntf/base/modelSimEngine.h"

namespace kathryn{

    class FlowBlockBase;
    struct Node;


    /**
     *
     * node is not include anymore
     */
    class FlowBlockBase;
    class FlowBaseSimEngine: public ModelProxyBuild,
                             public ModelProxyRetrieve{
    protected:
        FlowBlockBase* _flowBlockBase = nullptr;
        FlowColEle*    _writer        = nullptr;
    public:
        FlowBaseSimEngine(FlowBlockBase* flowBlockBase);
        void        proxyBuildInit() override{};
        std::string getVarName()     override;
        std::string getVarNameCurStatus();
        ull         getVarId()       override;

        std::string createVariable()   override;
        std::string createOp()         override; ///// we use op to do all operation
        std::string createMemorizeOp() override{return "";}
        std::string createMemBlkAssOp()override{return "";}
        bool        isUserDeclare()    override{return false;}

        ////////
        ///
        void        proxyRetInit(ProxySimEventBase* modelSimEvent)     override;
        ValRepBase* getProxyRep()      override;
        bool        isBlockRunning();


    };

    class FlowSimEngineInterface{
    public:
        virtual FlowBaseSimEngine* getSimEngine() = 0;
    };




}

#endif //KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H
