//
// Created by tanawin on 1/6/2024.
//

#include "proxyBuildMng.h"
#include <cstdlib>
#include <dlfcn.h>
#include <set>


namespace kathryn{

    ProxyBuildMng::~ProxyBuildMng(){
        unloadProxy();
    }

    std::vector<ModelProxyBuild*> ProxyBuildMng::doTopologySort(
        std::vector<ModelProxyBuild*>& graph){

        if (graph.empty()){return {};}

        struct MPD_META{
            ModelProxyBuild* mpd = nullptr;
            int nextDepIdx       = 0;
        };

        /** initialize variable*/
        std::set<ModelProxyBuild*>    visited;
        std::set<ModelProxyBuild*>    resulted;
        std::set<ModelProxyBuild*>    inGraph; ////// somedep is come from not related sorce
        std::vector<ModelProxyBuild*> result;
        std::stack<MPD_META>          dfs;
        ///////////////////////////////////////////////
        for (ModelProxyBuild* mdp: graph){ inGraph.insert(mdp);}


        for (auto ele: graph){
            assert(dfs.empty());
            dfs.push({ele, 0});

            while(!dfs.empty()){
                MPD_META& top = dfs.top();

                if (inGraph.find(top.mpd) == inGraph.end()){
                    dfs.pop(); ///// not in region of interest
                    continue;
                }

                bool isVisit  = visited .find(top.mpd) != visited.end();
                bool isResult = resulted.find(top.mpd) != resulted.end();


                if (top.nextDepIdx == 0){
                    ////////// first in in stack
                    if (isVisit){
                        mfAssert(isResult,
                            "cycle dep detect at node " + top.mpd->getVarName());
                        dfs.pop();
                        continue;
                    }
                    visited.insert(top.mpd);
                }


                if(top.nextDepIdx == top.mpd->getDep().size()){
                    //////////// last in stack
                    if (!isResult){
                        resulted.insert(top.mpd);
                        result.push_back(top.mpd);
                    }
                    dfs.pop();

                }else{ /////// in search
                    ModelProxyBuild* nextEle = top.mpd->getDep()[top.nextDepIdx];
                    dfs.push({nextEle, 0});
                    top.nextDepIdx++;
                }
            }
        }
        return result;
    }


    void ProxyBuildMng::setStartModule(Module* startModule){
        _startModule    = startModule;
        moduleSimEngine = startModule->getSimEngine();

    }



    void ProxyBuildMng::startWriteModelSim(){
        moduleSimEngine->proxyBuildInit();
        //// create file
        proxyfileWriter = new FileWriterBase(srcGenPath);
        //// create include
        proxyfileWriter->addData("#include \"../proxyEvent.h\"\n");

        /// create namespace
        ///
        proxyfileWriter->addData("namespace kathryn{\n\n\n\n\n\n");

        ///////// global variable
        startWriteCreateVariable();
        startWritePerfDec();
        ///////// start  Writefunction
        startWriteRegisterCallback();
        startWriteVcdDecVar(true);
        startWriteVcdDecVar(false);
        startWriteVolatileEleSim();
        startWriteNonVolatileEleSim();
        startWriteVcdCol(true);
        startWriteVcdCol(false);
        startWritePerfCol();
        startWriteCreateFunc();


        proxyfileWriter->addData("\n\n\n\n\n\n}\n");
        proxyfileWriter->flush();
        delete proxyfileWriter;

    }

    void ProxyBuildMng::startWriteCreateVariable(){
        assert(moduleSimEngine != nullptr);
        std::vector<ModelProxyBuild*> dayta =
            moduleSimEngine->recruitForCreateVar();

        for (ModelProxyBuild* mpb: dayta){
            proxyfileWriter->addData(mpb->createVariable());
        }
    }


    void ProxyBuildMng::startWritePerfDec(){
        std::vector<FlowBaseSimEngine*> dayta =
            moduleSimEngine->recruitPerf();

        proxyfileWriter->addData("/////////////////////// perf variable\n");

        for (ModelProxyBuild* mpb: dayta){
            proxyfileWriter->addData(mpb->createVariable());
        }

        proxyfileWriter->addData("/////////////////////// perf finish initialize\n");

    }



    void ProxyBuildMng::startWriteRegisterCallback(){

        auto registerHelper = [&](std::string varName, bool isPerf){
            assert(!varName.empty());
            proxyfileWriter->addData("       ");
            if (isPerf){
                proxyfileWriter->addData("registerToCallBackPerf(");
            }else{
                proxyfileWriter->addData("registerToCallBack(");
            }
            proxyfileWriter->addData("\"" + varName+ "\"");
            proxyfileWriter->addData(",");
            proxyfileWriter->addData(varName);
            proxyfileWriter->addData(");\n");
            //proxyfileWriter->addData(R"(std::cout << "register to " << ")" + varName + "\"" + " << std::endl; \n");

        };

        proxyfileWriter->addData("void ProxySimEvent::startRegisterCallBack(){\n");

        ///// for logic value
        for (ModelProxyBuild* mpb:
            moduleSimEngine->recruitForCreateVar()){
            registerHelper(mpb->getVarName(), false);
        }
        ///// for perf value     we are sure that name will not
        for (FlowBaseSimEngine* mpb: moduleSimEngine->recruitPerf()){
            std::vector<std::string> result;
            mpb->getRecurVarName(result);
            for (const std::string& varName: result){
                registerHelper(varName, true);
            }

        }

        proxyfileWriter->addData("}\n");
    }



    //////// non volatile must do TOPOLOGY SORT FIRST

    void ProxyBuildMng::startWriteVolatileEleSim(){

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


        proxyfileWriter->addData("void ProxySimEvent::startVolatileEleSim(){\n");
        std::vector<ModelProxyBuild*> mpbs = moduleSimEngine->recruitForOp();
        std::vector<ModelProxyBuild*> actual_mpbs = doTopologySort(mpbs);
        proxyfileWriter->addData("////////////////////// standard op\n");
        for (ModelProxyBuild* mpb: actual_mpbs){
            proxyfileWriter->addData(mpb->createOp());
        }
        proxyfileWriter->addData("}\n");

    }

    void ProxyBuildMng::startWriteNonVolatileEleSim(){

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

        proxyfileWriter->addData("void ProxySimEvent::startNonVolatileEleSim(){\n");

        std::vector<ModelProxyBuild*> mpbs = moduleSimEngine->recruitForOpEndCycle();

        proxyfileWriter->addData("////////////////////// standard op\n");
        for (ModelProxyBuild* mpb: mpbs){
            proxyfileWriter->addData(mpb->createOp());
        }
        proxyfileWriter->addData("//////////////////// transfer Op\n");
        for (ModelProxyBuild* mpb: mpbs){
            proxyfileWriter->addData(mpb->createOpEndCycle());
        }
        proxyfileWriter->addData("}\n");

    }


    void ProxyBuildMng::startWriteVcdDecVar(bool isUser){
        std::vector<LogicSimEngine*> dayta =
            moduleSimEngine->recruitAllLogicSimEngine();

        proxyfileWriter->addData("void ProxySimEvent::startVcdDecVar");
        proxyfileWriter->addData((isUser? "User": "Internal"));
        proxyfileWriter->addData("(){\n");


        for (LogicSimEngine* mpb: dayta){
            if (mpb->isUserDeclare() == isUser){ ////// the registerable must always put to vcd file
                proxyfileWriter->addData("       ");
                proxyfileWriter->addData("_vcdWriter->addNewVar(");

                //////// sigtype
                VCD_SIG_TYPE  vst = mpb->getSigType();
                if (vst == VST_REG){
                    proxyfileWriter->addData("VST_REG");
                }else if (vst == VST_WIRE){
                    proxyfileWriter->addData("VST_WIRE");
                }else if (vst == VST_INTEGER){
                    proxyfileWriter->addData("VST_INTEGER");
                }else{
                    assert(false);
                }

                /////// varname
                proxyfileWriter->addData(",");
                proxyfileWriter->addData("\""+ mpb->getVarName() +"\"");
                proxyfileWriter->addData(",");
                proxyfileWriter->addData("{" +
                    std::to_string(mpb->getSize().start) + "," +
                    std::to_string(mpb->getSize().stop)  + "});\n");
            }
        }
        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startWriteVcdCol(bool isUser){
        std::vector<LogicSimEngine*> dayta =
            moduleSimEngine->recruitAllLogicSimEngine();

        proxyfileWriter->addData("void ProxySimEvent::startVcdCol");
        proxyfileWriter->addData((isUser? "User": "Internal"));
        proxyfileWriter->addData("(){\n");


        for (ModelProxyBuild* mpb: dayta){
            if (mpb->isUserDeclare() == isUser){ ////// the registerable must always put to vcd file
                proxyfileWriter->addData("       ");
                proxyfileWriter->addData("_vcdWriter->addNewValue(");

                proxyfileWriter->addData("\""+ mpb->getVarName() +"\"");
                proxyfileWriter->addData(",");
                proxyfileWriter->addData("&" + mpb->getVarName() + ");\n");
            }
        }

        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startWritePerfCol(){
        std::vector<FlowBaseSimEngine*> dayta = moduleSimEngine->recruitPerf();

        proxyfileWriter->addData("void ProxySimEvent::startPerfCol");
        proxyfileWriter->addData("(){\n");


        for (ModelProxyBuild* mpb: dayta){
            proxyfileWriter->addData(mpb->createOp());
        }

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
            srcBuilderPath + " " + TEST_NAME;

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
        SeCreator create = (SeCreator) dlsym(_handle, "create");
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
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
