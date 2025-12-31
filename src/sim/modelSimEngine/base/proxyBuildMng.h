//
// Created by tanawin on 1/6/2024.
//

#ifndef PROXYBUILDMNG_H
#define PROXYBUILDMNG_H

#include <filesystem>
#include <string>
#include <cstdlib>
#include <utility>
#include "params/prjParam.h"
#include "frontEnd/cmd/paramReader.h"

#include "proxyEventBase.h"
#include "model/hwComponent/module/module.h"

#include "sim/modelSimEngine/flowBlock/flowBaseSim.h"
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"
#include "sim/modelSimEngine/hwComponent/module/moduleSim.h"
#include "util/fileWriter/fileWriterBase.h"
#include "modelProxy.h"
#include "traceEvent.h"
#include "userDefRepo.h"
#include "proxyBuildMode.h"


namespace kathryn{



    class ProxyBuildMng{
    protected:
        Module*          _startModule    = nullptr;
        ModuleSimEngine* moduleSimEngine = nullptr;
        FileWriterBase* proxyfileWriter  = nullptr;
        FileReaderBase* proxyfileReader  = nullptr;
        void*           _handle          = nullptr;
        UserDefRepo     _codeRepo; /// used to store old code

        const std::string TEST_NAME;
        const std::string INLINE_ATTR = "__attribute__((always_inline)) inline";
        const int         OP_LEVEL    = 3;
        const std::string OP_FLAG     = "-O3";

        //////// key of file and directory
        const std::string PROJECT_DIR    = "..";
        const std::string MD_COMPILE_FOLDER = "modelCompile";

        const std::string genFolder      = "generated";
        const std::string dynObjFolder   = "build";

        const std::string builderName    = "startGen.sh";

        ////// CODE_MAKER_TYPE match with trackKeys
        enum CODE_MAKER_TYPE{CMT_INCLUDE = 0, CMT_GLOBVAR = 1, CMT_MANUALDES = 2};
        const std::string trackKeys[3] = {"include", "globalVar", "manualDesigner"};




        //////// pathe of file and directory
        const std::string pathToModelFolder = PROJECT_DIR + "/" + MD_COMPILE_FOLDER;

        const std::string srcGenPath
        = pathToModelFolder + "/" + genFolder + "/" + TEST_NAME + ".cpp";
        const std::string srcBuilderPath
        = pathToModelFolder + "/" + builderName;
        const std::string srcDynLoadPath
        = pathToModelFolder + "/" + dynObjFolder + "/" + TEST_NAME +  ".so";

        /** function name*/
        ///////// [functionName][?userIdent][?ske suffix]
        const std::string BASE_CLASS_NAME = "ProxySimEvent";
        const std::string SKE_SUFFIX           = "Ske";
        const std::string USER_SUFFIX          = "User";
        const std::string INTERNAL_SUFFIX      = "Internal";
        const std::string INTERNAL_WARMUP      = "intCodeWarmUp";
        const std::string REGIS_CALLBACK       = "startRegisterCallBack";
        const std::string REGIS_CALLBACK_LOGIC = "registerToCallBack";
        const std::string REGIS_CALLBACK_PERF  = "registerToCallBackPerf";
        const std::string CLK_MODE_POSE        = "Pos";
        const std::string CLK_MODE_NEG         = "Neg";
        const std::string MAINOP_SIM = "startMainOpEleSim";
        const std::string FIZOP_SIM  = "startFinalizeEleSim";
        const std::string VCD_DEC    = "startVcdDecVar"; //// must have User or Internal as a suffix
        const std::string VCD_COL    = "startVcdCol";
        const std::string PERF_COL   = "startPerfCol";
        const std::string USER_DEF   = "userDef";
        const std::string MAIN_SIM   = "mainSim";


        /** variable and value name*/
        const std::string CALLBACK_VAR_ARR_NAME    = "kathrynCallBackMeta";
        const std::string MAX_SIZE_CB_ARR          = "MAX_PROX_CALLBACK_FUNCTION";
        const std::string CALLBACK_VAR_AMT         = "kathrynCallBackAmt";
        const std::string CALLBACK_GET_AMT         = "getCallBackAmt";
        const std::string CALLBACK_GET_NO          = "getCallBackNo";
        const std::string CALLBACK_CHECK_FUNC_NAME = "checkCallBack";
        const std::string VCD_WRITER_TYPE          = "VcdWriter";
        const std::string VCD_WRITER_VAR_INT       = "_vcdWriterInternal";
        const std::string VCD_WRITER_VAR_BASE_CL   = "_vcdWriter";

        std::vector<TraceEvent>*  callBackEvents = nullptr;

    public:
        explicit ProxyBuildMng(std::string testName,
                               bool reqInline = true,   ///  request function to  be inline
                               int  opLev = 3):      ///  optimization level
        TEST_NAME(std::move(testName)),
        INLINE_ATTR(reqInline ? "__attribute__((always_inline)) inline"
                              : ""),
        OP_LEVEL(opLev),
        OP_FLAG("-O" + std::to_string(opLev)),
        PROJECT_DIR(KATHRYN_PROJECT_DIR){

            mfAssert(opLev >= 0 && opLev <= 3,
                "invalid optimization level");

        }
        ~ProxyBuildMng();
        std::vector<ModelProxyBuild*>
        doTopologySort(std::vector<ModelProxyBuild*>& graph);

        std::string genFunctionDec(bool classRef, const std::string& funcName, const std::string& retType = "void");
        std::string genDummyFunctionFullDec(bool classRef, const std::string& funcName);

        void setStartModule(Module* startModule);
        void setTracer(std::vector<TraceEvent>* tracers);

        ////////// start recruit old system
        void startReadOldModelSim();
        ////////// generate path
        void startWriteModelSim();
        ///////// for wrate call back meta
        void startWriteCallBackVar();
        ////////// for create all variable
        void startWriteCreateVariable();
        ///////// void start write perf create
        void startWritePerfDec();
        ////////// start write internal warmup
        void startWriteInitInternalWarmUp();
        ////////// for start register function
        void startWriteRegisterCallback();
        ////////// call back function
        void startWriteCallBackVarInit();
        void startWriteCallBackCheckAndRet();
        void startWriteCallBackGetAmt();
        void startWriteCallbackGetNo();
        ///////// for create vcd Decvar
        void startWriteVcdDecWriter();
        void startWriteVcdDecVar(bool isUser); //// else if internal
        ///////// for create vcd Decvar
        void startWriteVcdCol(bool isUser);
        void startWriteVcdColSke(bool isUser);

        ///////// void start write perf col
        void startWritePerfColSke();
        void startWritePerfCol();
        ///// old system
        // void startMainOpEleSimSke();
        // void startMainOpEleSim   ();

        void startWriteAllLogicSim(CLOCK_MODE clkMode);

        ////////// for wire expression memElehodler*   etc....
        ///// volatile
        void startWriteMainEleSimSke  (CLOCK_MODE clkMode);
        void startWriteMainEleSim     (CLOCK_MODE clkMode);
        ///// non-volatile
        void startFinalizeEleSimSke(CLOCK_MODE clkMode);
        void startFinalizeEleSim   (CLOCK_MODE clkMode);
        ////////// for user define
        void startWriteUserDefinedFunction();
        //////// void start write for optimization
        void startWriteMainSimSke(bool userVcdCol,
                                  bool sysVcdCol,
                                  bool perfCol);

        void startWriteMainSim();
        //////// void start write creator
        void startWriteCreateFunc();

        //////// for screening clock mode
        std::vector<ModelProxyBuild*> screenClockMode(CLOCK_MODE clkMode,
                                                      std::vector<ModelProxyBuild*> srcs);
        std::string getClockModeStr(CLOCK_MODE clkMode);

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
