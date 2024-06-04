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
        FileWriterBase* fileWriter       = nullptr;
        const std::string desGenPath
        = "../src/modelCompile/generated/proxyEventUser.cpp";
        const std::string desCompilePath
        = "../src/modelCompile/generated/build/simClient.so";
        const std::string srcBuildPath
        ="../src/modelCompile/startGen.sh";

    public:
        ProxyBuildMng(Module* startModule);
        ~ProxyBuildMng();
        std::vector<ModelProxyBuild*>
        doTopologySort(std::vector<ModelProxyBuild*>& graph);

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



    };

}

#endif //PROXYBUILDMNG_H
