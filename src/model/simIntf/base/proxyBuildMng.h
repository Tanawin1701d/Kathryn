//
// Created by tanawin on 1/6/2024.
//

#ifndef PROXYBUILDMNG_H
#define PROXYBUILDMNG_H
#include <string>
#include <cstdlib>
#include <utility>

#include "proxyEventBase.h"
#include "model/hwComponent/module/module.h"
#include "model/simIntf/hwComponent/moduleSimEngine.h"
#include "util/fileWriter/fileWriterBase.h"



namespace kathryn{

    class ProxyBuildMng{
    protected:
        Module*          _startModule    = nullptr;
        ModuleSimEngine* moduleSimEngine = nullptr;
        FileWriterBase* proxyfileWriter  = nullptr;
        void*           _handle          = nullptr;

        const std::string TEST_NAME;

        const std::string PROJECT_DIR    = "..";
        const std::string MD_COMPILE_FOLDER = "modelCompile";

        const std::string genFolder      = "generated";
        const std::string dynObjFolder   = "build";

        const std::string builderName    = "startGen.sh";


        const std::string pathToModelFolder = PROJECT_DIR + "/" + MD_COMPILE_FOLDER;

        const std::string srcGenPath
        = pathToModelFolder + "/" + genFolder + "/" + TEST_NAME + ".cpp";
        const std::string srcBuilderPath
        = pathToModelFolder + "/" + builderName;
        const std::string srcDynLoadPath
        = pathToModelFolder + "/" + dynObjFolder + "/" + TEST_NAME +  ".so";

    public:
        ProxyBuildMng(std::string testName): TEST_NAME(std::move(testName)){};
        ~ProxyBuildMng();
        std::vector<ModelProxyBuild*>
        doTopologySort(std::vector<ModelProxyBuild*>& graph);

        void setStartModule(Module* startModule);

        ////////// generate path
        void startWriteModelSim();
        ////////// for create all variable
        void startWriteCreateVariable();
        ///////// void start write perf create
        void startWritePerfDec();
        ////////// for start register function
        void startWriteRegisterCallback();
        ////////// for wire expression memElehodler*   etc....
        void startMainOpEleSim();
        ////////// for register
        void startFinalizeEleSim();
        ///////// for create vcd Decvar
        void startWriteVcdDecVar(bool isUser); //// else if internal
        ///////// for create vcd Decvar
        void startWriteVcdCol(bool isUser);
        ///////// void start write perf col
        void startWritePerfCol();
        //////// void start write creator
        void startWriteCreateFunc();


        //////// compile file
        void startCompile();
        ///////// load path
        ProxySimEventBase* loadAndGetProxy();
        ///////// start retrieve data back
        void startRetrieveSimVal(ProxySimEventBase* simEvent);
        ///////// disload
        void unloadProxy();



    };

}

#endif //PROXYBUILDMNG_H
