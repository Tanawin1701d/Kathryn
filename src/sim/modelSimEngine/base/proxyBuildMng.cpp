//
// Created by tanawin on 1/6/2024.
//

#include "proxyBuildMng.h"

#include <cstdlib>
#include <dlfcn.h>
#include <set>
#include "params/simParam.h"


namespace kathryn{
    ProxyBuildMng::~ProxyBuildMng(){
        unloadProxy();
    }

    std::vector<ModelProxyBuild*> ProxyBuildMng::doTopologySort(
        std::vector<ModelProxyBuild*>& graph){
        if (graph.empty()){ return {}; }

        struct MPD_META{
            ModelProxyBuild* mpd = nullptr;
            int nextDepIdx = 0;
        };

        /** initialize variable*/
        std::set<ModelProxyBuild*> visited;
        std::set<ModelProxyBuild*> resulted;
        std::set<ModelProxyBuild*> inGraph; ////// somedep is come from not related sorce
        std::vector<ModelProxyBuild*> result;
        std::stack<MPD_META> dfs;
        ///////////////////////////////////////////////
        for (ModelProxyBuild* mdp : graph){ inGraph.insert(mdp); }


        for (auto ele : graph){
            assert(dfs.empty());
            dfs.push({ele, 0});

            while (!dfs.empty()){
                MPD_META& top = dfs.top();

                if (inGraph.find(top.mpd) == inGraph.end()){
                    dfs.pop(); ///// not in region of interest
                    continue;
                }

                bool isVisit = visited.find(top.mpd) != visited.end();
                bool isResult = resulted.find(top.mpd) != resulted.end();


                if (top.nextDepIdx == 0){
                    ////////// first in in stack
                    if (isVisit){
                        mfAssert(isResult,
                                 "cycle dep detect at node " + top.mpd->getValRep().getData());
                        dfs.pop();
                        continue;
                    }
                    visited.insert(top.mpd);
                }


                if (top.nextDepIdx == top.mpd->getDep().size()){
                    //////////// last in stack
                    if (!isResult){
                        resulted.insert(top.mpd);
                        result.push_back(top.mpd);
                    }
                    dfs.pop();
                }
                else{
                    /////// in search
                    ModelProxyBuild* nextEle = top.mpd->getDep()[top.nextDepIdx];
                    dfs.push({nextEle, 0});
                    top.nextDepIdx++;
                }
            }
        }
        return result;
    }

    std::string ProxyBuildMng::genFunctionDec(bool classRef,
                                              const std::string& funcName,
                                              const std::string& retType){
        return retType + " " + (classRef ? (BASE_CLASS_NAME + "::"): "") + funcName + "()";
    }

    std::string ProxyBuildMng::genDummyFunctionFullDec(bool classRef,
                                              const std::string& funcName){

        return genFunctionDec(classRef, funcName) +  "{}\n";

    }


    void ProxyBuildMng::setStartModule(Module* startModule){
        _startModule = startModule;
        moduleSimEngine = startModule->getSimEngine();
    }
    void ProxyBuildMng::setTracer(std::vector<TraceEvent>* tracers){
        assert(tracers != nullptr);
        callBackEvents = tracers;
    }

    void ProxyBuildMng::startReadOldModelSim(){

        for (const std::string& trackKey : trackKeys){
            _codeRepo.addTrackKey(trackKey);
        }

        if (!std::filesystem::exists(srcGenPath)){
            return;
        }

        proxyfileReader = new FileReaderBase(srcGenPath);
        _codeRepo.fetchCode(proxyfileReader);
        delete proxyfileReader;

    }


    void ProxyBuildMng::startWriteModelSim(){

        bool userVcd = (PARAM_VCD_REC_POL == MDE_REC_BOTH) ||
                       (PARAM_VCD_REC_POL == MDE_REC_ONLY_USER);

        bool internalVcd = (PARAM_VCD_REC_POL == MDE_REC_BOTH) ||
                           (PARAM_VCD_REC_POL == MDE_REC_ONLY_INTERNAL);

        bool perfCol      = PARAM_PERF_REC_POL == MFP_ON;


        moduleSimEngine->proxyBuildInit();
        /** create file */
        proxyfileWriter = new FileWriterBase(srcGenPath);
        /** write include*/
        proxyfileWriter->addData("#include \"../proxyEvent.h\"\n");
        /** write designer manual include*/
        proxyfileWriter->addData(_codeRepo.genCode(trackKeys[CMT_INCLUDE])+"\n"); //// include
        /** create namespace*/
        proxyfileWriter->addData("namespace kathryn{\n\n\n\n\n\n");

        ///////// global variable
        startWriteCallBackVarInit(); /// write variable for callback variable
        startWriteVcdDecWriter();    /// write vcd writer variable
        startWriteCreateVariable();  /// create variable for simulation model
        startWritePerfDec();         /// create variable for performance
        proxyfileWriter->addData(_codeRepo.genCode(trackKeys[CMT_GLOBVAR])+"\n"); /// globalvar
        //////////////////////////////
        ///////// start  Writefunction
        //////////////////////////////
        /// register callback
        startWriteInitInternalWarmUp(); //// warm-up vcd writer to bring from proxysim event
        startWriteRegisterCallback();   //// register the performance and simvalue to make userland simulatable
        //// callbackMng
        startWriteCallBackCheckAndRet(); //// callback checker
        startWriteCallBackGetAmt();      //// get amount of call back
        startWriteCallbackGetNo();
        /// vcd DecVar collect
        if (userVcd){
            startWriteVcdDecVar (true);
            startWriteVcdColSke (true);
            startWriteVcdCol    (true);
        }else{
            ///// no need to build ske
            proxyfileWriter->addData(genDummyFunctionFullDec(true, VCD_DEC + USER_SUFFIX));
            proxyfileWriter->addData(genDummyFunctionFullDec(true, VCD_COL + USER_SUFFIX));
        }
        if (internalVcd){
            startWriteVcdDecVar (false);
            startWriteVcdColSke (false);
            startWriteVcdCol    (false);
        }else{
            ///// no need to build ske
            proxyfileWriter->addData(genDummyFunctionFullDec(true, VCD_DEC + INTERNAL_SUFFIX));
            proxyfileWriter->addData(genDummyFunctionFullDec(true, VCD_COL + INTERNAL_SUFFIX));
        }
        /// perfcollection
        if (perfCol){
            startWritePerfColSke();
            startWritePerfCol();
        }else{
            proxyfileWriter->addData(genDummyFunctionFullDec(true, PERF_COL));
        }

        /// main op sim
        startMainOpEleSimSke();
        startMainOpEleSim();
        /// final op sim
        startFinalizeEleSimSke();
        startFinalizeEleSim();
        // user sim op
        startWriteUserDefinedFunction();
        /// main sim
        startWriteMainSimSke(userVcd, internalVcd, perfCol);
        startWriteMainSim();
        startWriteCreateFunc();
        /// w flush
        proxyfileWriter->addData("\n\n\n\n\n\n}\n");
        proxyfileWriter->flush();
        delete proxyfileWriter;
    }

    void ProxyBuildMng::startWriteCallBackVar(){
        proxyfileWriter->addData(CALLBACK_VAR_ARR_NAME + "["+ MAX_SIZE_CB_ARR+"];\n");
        proxyfileWriter->addData(CALLBACK_VAR_AMT +";\n");
    }

    void ProxyBuildMng::startWriteCreateVariable(){
        assert(moduleSimEngine != nullptr);
        /////// recruit modelBuilder for create variable
        std::vector<ModelProxyBuild*> dayta =
            moduleSimEngine->recruitForCreateVar();
        /////// generate the data
        CbBaseCxx cb;
        for (ModelProxyBuild* mpb : dayta){
            mpb->createGlobalVariable(cb);
        }
        ////////// write file
        proxyfileWriter->addData(cb.toString(0));
    }


    void ProxyBuildMng::startWritePerfDec(){
        std::vector<FlowBaseSimEngine*> dayta =
            moduleSimEngine->recruitPerf();

        proxyfileWriter->addData("/////////////////////// perf variable\n");

        CbBaseCxx cb;
        for (ModelProxyBuild* mpb : dayta){
            mpb->createGlobalVariable(cb);
        }
        proxyfileWriter->addData(cb.toString(0));
        proxyfileWriter->addData("/////////////////////// perf finish initialize\n");
    }

    void ProxyBuildMng::startWriteInitInternalWarmUp(){

        proxyfileWriter->addData(genFunctionDec(true, INTERNAL_WARMUP)+ "{\n");
        proxyfileWriter->addData(VCD_WRITER_VAR_INT + "=" + VCD_WRITER_VAR_BASE_CL  + ";\n");
        proxyfileWriter->addData("}\n");

    }

    void ProxyBuildMng::startWriteRegisterCallback(){


        auto registerHelper = [&](std::string varName, bool isPerf){
            assert(!varName.empty());
            proxyfileWriter->addData("       ");
            if (isPerf){
                proxyfileWriter->addData(REGIS_CALLBACK_PERF + "(");
            }
            else{
                proxyfileWriter->addData(REGIS_CALLBACK_LOGIC + "(");
            }
            proxyfileWriter->addData("\"" + varName + "\"");
            proxyfileWriter->addData(",");
            proxyfileWriter->addData(varName);
            proxyfileWriter->addData(");\n");
        };

        proxyfileWriter->addData(genFunctionDec(true, REGIS_CALLBACK)+ "{\n");

        ///// for logic value
        for (ModelProxyBuild* mpb :
             moduleSimEngine->recruitForRegisVar()){
            registerHelper(mpb->getValRep().getData(), false);
        }
        ///// for perf value     we are sure that name will not
        for (FlowBaseSimEngine* mpb : moduleSimEngine->recruitPerf()){
            std::vector<std::string> result;
            mpb->getRecurVarName(result);
            mpb->getRecurVarNameCurStsatus(result);
            for (const std::string& varName : result){
                registerHelper(varName, true);
            }
        }

        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startWriteCallBackVarInit(){
        proxyfileWriter->addData("int " + CALLBACK_VAR_AMT +"=0;\n");
        proxyfileWriter->addData("int " + CALLBACK_VAR_ARR_NAME +"["+
            MAX_SIZE_CB_ARR + "];\n");
    }

    void ProxyBuildMng::startWriteCallBackCheckAndRet(){

        proxyfileWriter->addData("__attribute__((always_inline)) inline bool " +
            CALLBACK_CHECK_FUNC_NAME + "(){\n");
        proxyfileWriter->addData("bool shouldRet = false;\n");

        for (int i = 0; i < callBackEvents->size(); i++ ){
            std::string condition = (*callBackEvents)[i].getCondStr();
            CbIfCxx x(false, condition);
            x.addSt("shouldRet = true");
            x.addSt(CALLBACK_VAR_ARR_NAME + "[" + CALLBACK_VAR_AMT + "] = " + std::to_string(i) );
            x.addSt(CALLBACK_VAR_AMT + "++");
            proxyfileWriter->addData(x.toString(0));
        }


        proxyfileWriter->addData("return shouldRet;\n");
        proxyfileWriter->addData("}\n");

    }

    void ProxyBuildMng::startWriteCallBackGetAmt(){
        proxyfileWriter->addData("int " + BASE_CLASS_NAME + "::" + CALLBACK_GET_AMT + "() const");
        proxyfileWriter->addData("{\n");
        proxyfileWriter->addData("return " + CALLBACK_VAR_AMT + ";\n");
        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startWriteCallbackGetNo(){
        proxyfileWriter->addData("int " + BASE_CLASS_NAME + "::" + CALLBACK_GET_NO + "(int idx) const");
        proxyfileWriter->addData("{\n");
        proxyfileWriter->addData("return " + CALLBACK_VAR_ARR_NAME +"[idx];\n");
        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startWriteVcdDecWriter(){
        proxyfileWriter->addData(VCD_WRITER_TYPE + "* " +
                                 VCD_WRITER_VAR_INT + "= nullptr;\n");
    }

    void ProxyBuildMng::startWriteVcdDecVar(bool isUser){
        std::vector<LogicSimEngine*> dayta =
            moduleSimEngine->recruitForVcdVar();

        std::string fSuffix = isUser ? USER_SUFFIX: INTERNAL_SUFFIX;
        proxyfileWriter->addData(genFunctionDec(true, VCD_DEC + fSuffix));
        proxyfileWriter->addData("{\n");


        for (LogicSimEngine* mpb : dayta){
            if (mpb->isUserDeclare() == isUser){
                ////// the registerable must always put to vcd file
                proxyfileWriter->addData("       ");
                proxyfileWriter->addData("_vcdWriter->addNewVar(");

                //////// sigtype
                VCD_SIG_TYPE vst = mpb->getSigType();
                if (vst == VST_REG){
                    proxyfileWriter->addData("VST_REG");
                }
                else if (vst == VST_WIRE){
                    proxyfileWriter->addData("VST_WIRE");
                }
                else if (vst == VST_INTEGER){
                    proxyfileWriter->addData("VST_INTEGER");
                }
                else{
                    assert(false);
                }

                /////// varname
                proxyfileWriter->addData(",");
                proxyfileWriter->addData("\"" + mpb->getValRep().getData() + "\"");
                proxyfileWriter->addData(",");
                proxyfileWriter->addData("{" +
                    std::to_string(mpb->getSize().start) + "," +
                    std::to_string(mpb->getSize().stop) + "});\n");
            }
        }
        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startWriteVcdColSke(bool isUser){
        std::vector<LogicSimEngine*> dayta =
            moduleSimEngine->recruitForVcdVar();

        std::string fSuffix = isUser ? USER_SUFFIX: INTERNAL_SUFFIX;
        proxyfileWriter->addData("__attribute__((always_inline)) inline " +
            genFunctionDec(false, VCD_COL + fSuffix + SKE_SUFFIX));
        proxyfileWriter->addData("{\n");
        //proxyfileWriter->addData("}\n");
        //return;

        for (LogicSimEngine* mpb : dayta){
            if (mpb->isUserDeclare() == isUser){
                ////// the registerable must always put to vcd file
                proxyfileWriter->addData("       ");
                proxyfileWriter->addData(VCD_WRITER_VAR_INT + "->addNewValue(");

                proxyfileWriter->addData("\"" + mpb->getValRep().getData() + "\"");
                proxyfileWriter->addData(",");
                if (mpb->getValR_Type().type == SVT_U64M){
                    //proxyfileWriter->addData("0");
                    proxyfileWriter->addData(mpb->getValRep().getData() + ".toBiStr()");
                }else{
                    proxyfileWriter->addData(mpb->getValRep().getData());
                }
                proxyfileWriter->addData(");\n");
            }
        }

        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startWriteVcdCol(bool isUser){

        std::string fSuffix = isUser ? USER_SUFFIX: INTERNAL_SUFFIX;
        proxyfileWriter->addData(genFunctionDec(true, VCD_COL + fSuffix ));

        proxyfileWriter->addData("{\n");
        proxyfileWriter->addData("      " + VCD_COL + fSuffix + SKE_SUFFIX);
        proxyfileWriter->addData("();\n");
        proxyfileWriter->addData("}\n");

    }

    void ProxyBuildMng::startWritePerfColSke(){
        std::vector<FlowBaseSimEngine*> dayta = moduleSimEngine->recruitPerf();

        proxyfileWriter->addData("__attribute__((always_inline)) inline " +
            genFunctionDec(false, PERF_COL+  SKE_SUFFIX));
        proxyfileWriter->addData("{\n");

        CbBaseCxx cb;
        for (ModelProxyBuild* mpb : dayta){
            mpb->createOp(cb);
        }
        proxyfileWriter->addData(cb.toString(0));
        proxyfileWriter->addData("}\n");
    }


    void ProxyBuildMng::startWritePerfCol(){
        proxyfileWriter->addData(genFunctionDec(true, PERF_COL) + "{\n");
        proxyfileWriter->addData("      startPerfColSke();\n");
        proxyfileWriter->addData("}\n");

    }


    //////// non volatile must do TOPOLOGY SORT FIRST

    void ProxyBuildMng::startMainOpEleSimSke(){
        /***
         *
         * 1.standard op
         * 2.memAssign memnt
         * 3.transfer op
         *
         */
        ///////////// do not worry about register simulation will get false new
        ///data from memory if there is update from memory to register because
        /// memEleHolder will provide temporary data to register simulation

        std::vector<ModelProxyBuild*> volatileEle    = moduleSimEngine->recruitForMainOpVolatile();
        std::vector<ModelProxyBuild*> nonVolatileEle = moduleSimEngine->recruitForMainOpNonVolatile();

        proxyfileWriter->addData("__attribute__((always_inline)) inline " +
                                 genFunctionDec(false, MAINOP_SIM + SKE_SUFFIX) +
                                 "{\n");

        //////////////////////// create local variable
        CbBaseCxx cbInitVal;
        for (ModelProxyBuild* mpb : volatileEle){
            mpb->createLocalVariable(cbInitVal);
        }
        for (ModelProxyBuild* mpb : nonVolatileEle){
            mpb->createLocalVariable(cbInitVal);
        }
        proxyfileWriter->addData(cbInitVal.toString(0));
        //////////////////////// volatile sim
        proxyfileWriter->addData("//// do main sim volatile\n");
        CbBaseCxx cbVolatileSim;
        std::vector<ModelProxyBuild*> sortedVolatileEle = doTopologySort(volatileEle);
        for (ModelProxyBuild* mpb : sortedVolatileEle){
            mpb->createOp(cbVolatileSim);
        }
        proxyfileWriter->addData(cbVolatileSim.toString(0));
        //////////////////////// nonVolatile sim
        proxyfileWriter->addData("//// do main sim NON volatile\n");
        CbBaseCxx cbNonVolatileSim;
        for (ModelProxyBuild* mpb : nonVolatileEle){
            /////// don't have to sort
            mpb->createOp(cbNonVolatileSim);
        }
        proxyfileWriter->addData(cbNonVolatileSim.toString(0));

        proxyfileWriter->addData("}\n");


    }

    void ProxyBuildMng::startMainOpEleSim(){
        proxyfileWriter->addData(genFunctionDec(true, MAINOP_SIM) + "{\n");
        proxyfileWriter->addData(MAINOP_SIM + SKE_SUFFIX + "();\n");
        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startFinalizeEleSimSke(){
        /***
         *
         * 1.standard op
         * 2.memAssign memnt
         * 3.transfer op
         *
         */

        ///////////// do not worry about register simulation will get false new
        ///data from memory if there is update from memory to register because
        /// memEleHolder will provide temporary data to register simulation

        proxyfileWriter->addData("__attribute__((always_inline)) inline "+
                                 genFunctionDec(false, FIZOP_SIM + SKE_SUFFIX)+
                                 "{\n");

        std::vector<ModelProxyBuild*> mpbs = moduleSimEngine->recruitForFinalizeOp();

        //////////////////// priority 1 ///////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////
        CbBaseCxx cbFinal1; ///// typically memOp
        proxyfileWriter->addData("////////////////////// transfer  op priority 1\n");
        for (ModelProxyBuild* mpb : mpbs){
            mpb->createOpEndCycle(cbFinal1);
        }
        proxyfileWriter->addData(cbFinal1.toString(0));
        //////////////////// priority 2 ///////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////
        CbBaseCxx cbFinal2; ///// typically memOp
        proxyfileWriter->addData("//////////////////// transfer Op priority 2\n");
        for (ModelProxyBuild* mpb : mpbs){
            mpb->createOpEndCycle2(cbFinal2);

        }
        proxyfileWriter->addData(cbFinal2.toString(0));

        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startFinalizeEleSim(){
        proxyfileWriter->addData(genFunctionDec(true, FIZOP_SIM) + "{\n");
        proxyfileWriter->addData(FIZOP_SIM + SKE_SUFFIX +"();\n");
        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startWriteUserDefinedFunction(){

        proxyfileWriter->addData("__attribute__((always_inline)) inline "+
                                 genFunctionDec(false, USER_DEF + USER_SUFFIX + SKE_SUFFIX)+
                                 "{\n");
        proxyfileWriter->addData("\n\n\n\n\n\n\n\n\n\n\n");
        assert(moduleSimEngine != nullptr);
        /////// recruit modelBuilder for create user mark value
        std::vector<ModelProxyBuild*> dayta =
            moduleSimEngine->recruitForCreateVar();
        /////// generate the data
        CbBaseCxx cb;
        for (ModelProxyBuild* mpb : dayta){
            mpb->createUserMarkValue(cb);
        }
        ////////// write the designer
        proxyfileWriter->addData(cb.toString(0));
        proxyfileWriter->addData(_codeRepo.genCode(trackKeys[CMT_MANUALDES])+"\n"); /// manualDesigner
        proxyfileWriter->addData("}\n");

    }


    void ProxyBuildMng::startWriteMainSimSke(bool userVcdCol,
                                             bool sysVcdCol,
                                             bool perfCol){
        proxyfileWriter->addData("CYCLE " + MAIN_SIM + SKE_SUFFIX + "(CYCLE kathryn_longrangeLim){\n");
        proxyfileWriter->addData("CYCLE kathryn_longrangeCnt  = 0;\n");
        proxyfileWriter->addData("do{\n");
        /////// TODO add tricker Event
        proxyfileWriter->addData(USER_DEF   + USER_SUFFIX + SKE_SUFFIX + "();\n");
        proxyfileWriter->addData(MAINOP_SIM + SKE_SUFFIX + "();\n");
        if (userVcdCol){ proxyfileWriter->addData(VCD_COL + USER_SUFFIX     + SKE_SUFFIX + "();\n");}
        if (sysVcdCol) { proxyfileWriter->addData(VCD_COL + INTERNAL_SUFFIX + SKE_SUFFIX + "();\n");}
        proxyfileWriter->addData(FIZOP_SIM + SKE_SUFFIX + "();\n");
        if (perfCol){ proxyfileWriter->addData(PERF_COL + SKE_SUFFIX + "();\n");}
        proxyfileWriter->addData("kathryn_longrangeCnt++;\n");
        proxyfileWriter->addData("}"); //// out of while loop
        proxyfileWriter->addData("while(!" + CALLBACK_CHECK_FUNC_NAME + "() && "
                                "(kathryn_longrangeCnt < kathryn_longrangeLim)"
                                                                        ");\n");
        proxyfileWriter->addData("return kathryn_longrangeCnt;\n");
        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startWriteMainSim(){
        proxyfileWriter->addData("CYCLE " + BASE_CLASS_NAME + "::"+ MAIN_SIM + "(){\n");
        proxyfileWriter->addData(CALLBACK_VAR_AMT + " = 0;\n");
        proxyfileWriter->addData( "return " + MAIN_SIM + SKE_SUFFIX + "(_amtLimitLongRangeCycle);\n");
        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startWriteCreateFunc(){
        proxyfileWriter->addData(
            "extern \"C\" ProxySimEventBase* create() {\n"
#ifdef MODELCOMPILEVB
        "   std::cout << \"creating proxy simEvent in dynamic object\" << std::endl;\n"
#endif
            "   return new ProxySimEvent();\n}\n\n"
        );
    }


    void ProxyBuildMng::startCompile(){
        std::string compileComand =
            srcBuilderPath + " " + TEST_NAME + " " + PROJECT_DIR;

#ifdef MODELCOMPILEVB
            std::cout << "compile command is " << compileComand << std::endl;
#endif
        int result = system(compileComand.c_str());
#ifdef MODELCOMPILEVB
        if (result == 0){
            std::cout << "compile successfully\n";
        }else{
            std::cerr << "Compilation failed with error code " << result << std::endl;
        }
#endif
    }

    ProxySimEventBase* ProxyBuildMng::loadAndGetProxy(){
        //////// open dynamic link
        _handle = dlopen(srcDynLoadPath.c_str(), RTLD_LAZY);
        const char* dlopen_error = dlerror();
        if (dlopen_error){
            std::cerr << "can't dynamic open " << dlopen_error << std::endl;
            exit(EXIT_FAILURE);
        }

        /////// dynamic load symbol
        typedef ProxySimEventBase* (*SeCreator)();
        SeCreator create = (SeCreator)dlsym(_handle, "create");
        const char* dlsym_error = dlerror();
        if (dlsym_error){
            std::cerr << "can't load symbol 'create': " << dlsym_error << std::endl;
            dlclose(_handle);
            exit(EXIT_FAILURE);
        }
        /////// create object
        ProxySimEventBase* pixieEvent = create();
        //pixieEvent->eventWarmUp();
        //pixieEvent->startVcdDecVarInternal();
        return pixieEvent;
    }

    void ProxyBuildMng::startRetrieveSimVal(ProxySimEventBase* simEvent){
        assert(simEvent != nullptr);
        assert(_startModule != nullptr);
        moduleSimEngine->retrieveInit(simEvent);
    }

    void ProxyBuildMng::unloadProxy(){
        int closeStatus = dlclose(_handle);
        assert(closeStatus == 0);
    }

}
