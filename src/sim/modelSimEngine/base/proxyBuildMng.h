//
// Created by tanawin on 1/6/2024.
//

#ifndef PROXYBUILDMNG_H
#define PROXYBUILDMNG_H
#include <string>
#include <cstdlib>
#include <utility>
#include <frontEnd/cmd/paramReader.h>

#include "proxyEventBase.h"
#include "model/hwComponent/module/module.h"

#include "sim/modelSimEngine/flowBlock/flowBaseSim.h"
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"
#include "sim/modelSimEngine/hwComponent/module/moduleSim.h"
#include "util/fileWriter/fileWriterBase.h"
#include "modelProxy.h"
#include "traceEvent.h"


namespace kathryn{



    class ProxyBuildMng{
    protected:
        Module*          _startModule    = nullptr;
        ModuleSimEngine* moduleSimEngine = nullptr;
        FileWriterBase* proxyfileWriter  = nullptr;
        void*           _handle          = nullptr;

        const std::string TEST_NAME;

        //////// key of file and directory
        const std::string PROJECT_DIR    = "..";
        const std::string MD_COMPILE_FOLDER = "modelCompile";

        const std::string genFolder      = "generated";
        const std::string dynObjFolder   = "build";

        const std::string builderName    = "startGen.sh";

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
        explicit ProxyBuildMng(std::string testName):
        TEST_NAME(std::move(testName)){};
        ~ProxyBuildMng();
        std::vector<ModelProxyBuild*>
        doTopologySort(std::vector<ModelProxyBuild*>& graph);

        std::string genFunctionDec(bool classRef, const std::string& funcName, const std::string& retType = "void");
        std::string genDummyFunctionFullDec(bool classRef, const std::string& funcName);

        void setStartModule(Module* startModule);
        void setTracer(std::vector<TraceEvent>* tracers);

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
        ////////// for wire expression memElehodler*   etc....
        void startMainOpEleSimSke();
        void startMainOpEleSim();
        ////////// for register
        void startFinalizeEleSimSke();
        void startFinalizeEleSim();
        ////////// for user define
        void startWriteUserDefinedFunction();
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


    //////// run mode

    enum class SimProxyBuildMode: uint8_t{
        SPB_NON         = 0,
        SPB_GEN         = 1 << 0,
        SPB_COMPILE     = 1 << 1,
        SPB_RUN         = 1 << 2
    };

    inline SimProxyBuildMode
    operator | (SimProxyBuildMode a, SimProxyBuildMode b){
        return static_cast<SimProxyBuildMode>(
            static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
        );
    }

    inline SimProxyBuildMode
    operator & (SimProxyBuildMode a, SimProxyBuildMode b){
        return static_cast<SimProxyBuildMode>(
            static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
        );
    }
    ////// has config b
    inline bool hasConfig(SimProxyBuildMode a, SimProxyBuildMode b){
        return (a & b) == b;
    }


    constexpr char param_spb_key[] = "buildMode";
    constexpr char param_spb_g     = 'g';
    constexpr char param_spb_c     = 'c';
    constexpr char param_spb_r     = 'r';

    SimProxyBuildMode getSPBM(const PARAM& param);

}

#endif //PROXYBUILDMNG_H
