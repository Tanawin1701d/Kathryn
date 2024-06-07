//
// Created by tanawin on 31/5/2024.
//

#ifndef MODEL_SIMITF_HWCOMPONENT_MODULESIMENGINE_H
#define MODEL_SIMITF_HWCOMPONENT_MODULESIMENGINE_H
#include <model/simIntf/base/modelProxy.h>
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
        std::vector<ModelProxyBuild*> recruitForCreateVar         ();
        std::vector<ModelProxyBuild*> recruitForVolatileEle       ();
        std::vector<ModelProxyBuild*> recruitForNonVolatileEle    ();
        std::vector<LogicSimEngine*>  recruitAllLogicSimEngine    (); ///// for vcdWrite
        /** create zone for perf element*/
        std::vector<ModelProxyBuild*> recruitPerf                 ();

        void recruitFromRegable(std::vector<ModelProxyBuild*>& result);
        void recruitFromWireable(std::vector<ModelProxyBuild*>& result); /// memblock is not include

        /** retrieve zone*/
        void retrieveInit(ProxySimEventBase* simEventBase);

        /** template zone*/
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
