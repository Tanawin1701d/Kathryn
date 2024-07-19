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

#include "sim/modelSimEngine/flowBlock/flowBaseSim.h"
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"
#include "sim/modelSimEngine/hwComponent/module/moduleSim.h"
#include "util/fileWriter/fileWriterBase.h"
#include "modelProxy.h"





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

        ///////// [functionName][?userIdent][?ske suffix]
        const std::string BASE_CLASS_NAME = "ProxySimEvent";
        const std::string SKE_SUFFIX           = "Ske";
        const std::string USER_SUFFIX          = "User";
        const std::string INTERNAL_SUFFIX      = "Internal";
        const std::string REGIS_CALLBACK       = "startRegisterCallBack";
        const std::string REGIS_CALLBACK_LOGIC = "startRegisterCallBack";
        const std::string REGIS_CALLBACK_PERF  = "registerToCallBackPerf";
        const std::string MAINOP_SIM = "startMainOpEleSim";
        const std::string FIZOP_SIM  = "startFinalizeEleSim";
        const std::string VCD_DEC    = "startVcdDecVar"; //// must have User or Internal as a suffix
        const std::string VCD_COL    = "startVcdCol";
        const std::string PERF_COL   = "startPerfCol";
        const std::string MAIN_SIM   = "mainSim";

    public:
        ProxyBuildMng(std::string testName): TEST_NAME(std::move(testName)){};
        ~ProxyBuildMng();
        std::vector<ModelProxyBuild*>
        doTopologySort(std::vector<ModelProxyBuild*>& graph);

        std::string genFunctionDec(bool classRef, const std::string& funcName);

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
        void startMainOpEleSimSke();
        void startMainOpEleSim();
        ////////// for register
        void startFinalizeEleSimSke();
        void startFinalizeEleSim();
        ///////// for create vcd Decvar
        void startWriteVcdDecVar(bool isUser); //// else if internal
        ///////// for create vcd Decvar
        void startWriteVcdCol(bool isUser);
        void startWriteVcdColSke(bool isUser);
        ///////// void start write perf col
        void startWritePerfColSke();
        void startWritePerfCol();
        //////// void start write for optimization
        void startWriteMainSimSke(bool userVcdCol,
                                  bool sysVcdCol,
                                  bool perfCol);
        void startWriteMainSim();
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
