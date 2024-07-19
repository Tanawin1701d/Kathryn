//
// Created by tanawin on 31/5/2024.
//

#ifndef MODEL_SIMITF_HWCOMPONENT_MODULESIMENGINE_H
#define MODEL_SIMITF_HWCOMPONENT_MODULESIMENGINE_H
#include "sim/modelSimEngine/base/modelProxy.h"
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"
#include "sim/modelSimEngine/flowBlock/flowBaseSim.h"

#include "util/fileWriter/fileWriterBase.h"


namespace kathryn{

    class Module;
    class LogicSimEngine;
    class ModuleSimEngine{
    protected:
        Module*         _module = nullptr;
    public:
        explicit ModuleSimEngine(Module* module); //// todo module
        /** create zone for hardware element*/
        void                          proxyBuildInit              ();
        std::vector<ModelProxyBuild*> recruitForCreateVar         ();
        std::vector<ModelProxyBuild*> recruitForRegisVar          (); ////// register
        std::vector<ModelProxyBuild*> recruitForMainOpVolatile    (); ///// must do topologysort
        std::vector<ModelProxyBuild*> recruitForMainOpNonVolatile ();
        std::vector<ModelProxyBuild*> recruitForFinalizeOp        ();
        std::vector<LogicSimEngine*>  recruitForVcdVar            (); ////// register
        //std::vector<LogicSimEngine*>  recruitAllLogicSimEngine    (); ///// for vcdWrite
        /** create zone for perf element*/
        std::vector<FlowBaseSimEngine*> recruitPerf             ();

        ////// stateFull u stateLess u memBlk u memElh(R) u memElh(W)
        void recruitStateFullEle(std::vector<ModelProxyBuild*>& result);
        void recruitStateLessEle(std::vector<ModelProxyBuild*>& result);
        void recruitMemBlk      (std::vector<ModelProxyBuild*>& result);
        void recruitMemElh      (std::vector<ModelProxyBuild*>& result, bool isReadMode);

        void recruitFromSpReg   (std::vector<ModelProxyBuild*>& result);



        /** retrieve zone*/
        void retrieveInit(ProxySimEventBase* simEventBase);

        /** template zone*/
        template<typename T>
        void recruitFromSubModule(std::vector<T*>& result, std::vector<T*> (ModuleSimEngine::*func)());

        template<typename S, typename T>
        void recruitFromVector(
            std::vector<S*>& result,
            std::vector<T>& eleVec);

        template<typename T>
        void retrieveInitFromVector(ProxySimEventBase* simEventBase,std::vector<T*>& eleVec);


    };

    class ModuleSimEngineInterface{
    public:
        virtual ~ModuleSimEngineInterface() = default;
        virtual ModuleSimEngine* getSimEngine() = 0;
    };

}

#endif //MODEL_SIMITF_HWCOMPONENT_MODULESIMENGINE_H
