//
// Created by tanawin on 1/6/2024.
//

#ifndef PROXYBUILDMNG_H
#define PROXYBUILDMNG_H
#include <string>
#include "model/hwComponent/module/module.h"
#include "model/simIntf/hwComponent/moduleSimEngine.h"
#include "util/fileWriter/fileWriterBase.h"


namespace kathryn{

    class ProxyBuildMng{
    protected:
        Module* _startModule = nullptr;
        ModuleSimEngine* moduleSimEngine = nullptr;
        FileWriterBase* fileWriter = nullptr;
        const std::string desPath
        = "../src/modelCompile/proxyEventUser.cpp";

        const std::string PERF_PREFIX = "PERF_";

    public:
        ProxyBuildMng(Module* startModule);
        ~ProxyBuildMng();
        std::vector<ModelProxyBuild*>
        doTopologySort(std::vector<ModelProxyBuild*>& graph);

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


    };

}

#endif //PROXYBUILDMNG_H
