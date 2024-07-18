//
// Created by tanawin on 29/3/2567.
//



#ifndef KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H
#define KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H

#include "sim/modelSimEngine/base/modelProxy.h"
#include "sim/simResWriter/simResWriter.h"

namespace kathryn{

    class FlowBlockBase;
    struct Node;


    /**
     *
     * node is not include anymore
     */

    constexpr int SUB_FLOWBLOCK_GEN_OP_SPACE = 4;

    class FlowBlockBase;
    class FlowBaseSimEngine: public ModelProxyBuild,
                             public ModelProxyRetrieve{
    protected:
        FlowBlockBase* _flowBlockBase  = nullptr;
        FlowColEle*    _writer         = nullptr;
        int            _opSpace        = 0;
        ValRepBase*    _proxyRepCurBit = nullptr;

    public:
        FlowBaseSimEngine(FlowBlockBase* flowBlockBase);
        ~FlowBaseSimEngine() override;
        void                     proxyBuildInit() override{};
        std::string              getVarName()     override;
        std::vector<std::string> getRegisVarName() override;
        std::string              getVarNameCurStatus();
        ull                      getVarId()       override;
        void                     getRecurVarName(std::vector<std::string>& result);
        void                     getRecurVarNameCurStsatus(std::vector<std::string>& result);

        std::string createGlobalVariable()   override;
        std::string createLocalVariable()    override{return "";}
        std::string createOp()               override; ///// we use op to do all operation
        std::string createOpEndCycle()       override{return "";}
        std::string createOpEndCycle2()      override{return "";}
        bool        isUserDeclare()          override{return false;}

        void        setOpSpace(int space){_opSpace = space; assert(space >= 0);}

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
