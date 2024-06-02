//
// Created by tanawin on 31/5/2024.
//

#ifndef MODEL_SIMITF_HWCOMPONENT_MODULESIMENGINE_H
#define MODEL_SIMITF_HWCOMPONENT_MODULESIMENGINE_H
#include <model/simIntf/base/modelProxy.h>
#include "util/fileWriter/fileWriterBase.h"


namespace kathryn{

    class Module;
    class ModuleSimEngine{
    protected:
        Module*         _module = nullptr;
    public:
        explicit ModuleSimEngine(Module* module); //// todo module
        /** create zone*/
        std::vector<ModelProxyBuild*> recruitForCreateVar         ();
        std::vector<ModelProxyBuild*> recruitForVolatileEle       ();
        std::vector<ModelProxyBuild*> recruitForNonVolatileEle    ();

        void recruitFromRegable(std::vector<ModelProxyBuild*>& result);
        void recruitFromWireable(std::vector<ModelProxyBuild*>& result); /// memblock is not include

        template<typename T>
        void recruitFromVector(
            std::vector<ModelProxyBuild*>& result,
            std::vector<T>& eleVec);

        ////// TODO for register


    };

    class ModuleSimEngineInterface{
    public:
        virtual ~ModuleSimEngineInterface() = 0;
        virtual ModuleSimEngine* getSimEngine() = 0;
    };

}

#endif //MODEL_SIMITF_HWCOMPONENT_MODULESIMENGINE_H
