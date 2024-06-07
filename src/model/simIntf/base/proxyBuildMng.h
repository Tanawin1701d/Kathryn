//
// Created by tanawin on 1/6/2024.
//

#ifndef PROXYBUILDMNG_H
#define PROXYBUILDMNG_H
#include <string>
#include <cstdlib>

#include "model/hwComponent/module/module.h"
#include "model/simIntf/hwComponent/moduleSimEngine.h"
#include "util/fileWriter/fileWriterBase.h"
#include "modelCompile/proxyEventBase.h"


namespace kathryn{

    class ProxyBuildMng{
    protected:
        Module*          _startModule    = nullptr;
        ModuleSimEngine* moduleSimEngine = nullptr;
        FileWriterBase* proxyfileWriter  = nullptr;
        void*           _handle          = nullptr;


        const std::string srcGenPath
        = "../src/modelCompile/generated/proxyEventUser.cpp";
        const std::string srcBuilderPath
        = "../src/modelCompile/startGen.sh";
        const std::string srcDynLoadPath
        = "../src/modelCompile/build/simClient.so";

    public:
        ProxyBuildMng() = default;
        ~ProxyBuildMng();
        std::vector<ModelProxyBuild*>
        doTopologySort(std::vector<ModelProxyBuild*>& graph);

        void setStartModule(Module* startModule);

        ////////// generate path
        void startWriteModelSim();
        ////////// for create all variable
        void startWriteCreateVariable();
        ////////// for start register function
        void startWriteRegisterCallback();
        ////////// for wire expression memElehodler*   etc....
        void startWriteVolatileEleSim();
        ////////// for register
        void startWriteNonVolatileEleSim();
        ///////// for create vcd Decvar
        void startWriteVcdDecVar(bool isUser); //// else if internal
        ///////// for create vcd Decvar
        void startWriteVcdCol(bool isUser);
        ///////// void start write perf create
        void startWritePerfDec();
        ///////// void start write perf col
        void startWritePerfCol();
        //////// compile file
        void startCompile();
        ///////// load path
        ProxySimEventBase* loadAndGetProxy();
        ///////// disload
        void unloadProxy();



    };

}

#endif //PROXYBUILDMNG_H
