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
        ValRepBase     _proxyRepCurBit;

    public:
        FlowBaseSimEngine(FlowBlockBase* flowBlockBase);
        ~FlowBaseSimEngine() = default;
        void                     proxyBuildInit() override{};
        ValR                     getValRep()     override;
        std::vector<std::string> getRegisVarName() override;
        ValR                     getVarNameCurStatus();
        ull                      getVarId()       override;
        SIM_VALREP_TYPE_ALL      getValR_Type()   override;
        CLOCK_MODE               getClockMode()   override;
        void                     getRecurVarName(std::vector<std::string>& result);
        void                     getRecurVarNameCurStsatus(std::vector<std::string>& result);

        void createGlobalVariable (CbBaseCxx& cb) override;
        void createLocalVariable  (CbBaseCxx& cb) override{}
        void createOp             (CbBaseCxx& cb) override; ///// we use op to do all operation
        void createOpEndCycle     (CbBaseCxx& cb) override{}
        void createOpEndCycle2    (CbBaseCxx& cb) override{}
        void createUserMarkValue  (CbBaseCxx& cb) override{}
        bool        isUserDeclare()          override{return false;}

        ////////
        ///
        void        proxyRetInit(ProxySimEventBase* modelSimEvent)     override;
        ValRepBase& getProxyRep()      override;
        bool        isBlockRunning();


    };

    class FlowSimEngineInterface{
    public:
        virtual FlowBaseSimEngine* getSimEngine() = 0;
    };




}

#endif //KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H
