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
        delete proxyfileWriter;
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
            assert(!dfs.empty());
            dfs.push({ele, 0});

            while(!dfs.empty()){
                MPD_META& top = dfs.top();

                if (inGraph.find(top.mpd) == inGraph.end()){
                    dfs.pop();
                    continue;
                }

                bool isVisit  = visited .find(top.mpd) != visited.end();
                bool isResult = resulted.find(top.mpd) != resulted.end();
                if (isVisit && isResult){
                    dfs.pop();
                    continue;
                }
                mfAssert(isVisit & (~isResult), "circle topology sort detected");

                if(top.nextDepIdx == 0){ /// first time add
                    visited.insert(top.mpd);
                }
                if(top.nextDepIdx == top.mpd->getDep().size()){
                    resulted.insert(top.mpd);
                    result.push_back(top.mpd);
                    dfs.pop();
                    continue;
                }

                ModelProxyBuild* nextEle = top.mpd->getDep()[top.nextDepIdx];
                dfs.push({nextEle, 0});
                top.nextDepIdx++;

            }
        }
        return result;
    }


    void ProxyBuildMng::setStartModule(Module* startModule){
        _startModule    = startModule;
        moduleSimEngine = startModule->getSimEngine();

    }



    void ProxyBuildMng::startWriteModelSim(){

        //// create file
        proxyfileWriter = new FileWriterBase(srcGenPath);
        //// create include
        proxyfileWriter->addData("#include \"src/modelCompile/proxyEvent.h\"\n");

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
            proxyfileWriter->addData("\n");
        }
    }

    void ProxyBuildMng::startWriteRegisterCallback(){
        std::vector<ModelProxyBuild*> dayta =
            moduleSimEngine->recruitForCreateVar();

        proxyfileWriter->addData("void ProxySimEvent::startRegisterCallBack(){\n");


        for (ModelProxyBuild* mpb: dayta){
                proxyfileWriter->addData("       ");
                proxyfileWriter->addData("registerToCallBack(");
                proxyfileWriter->addData("\"" + mpb->getVarName()+ "\"");
                proxyfileWriter->addData(",");
                proxyfileWriter->addData(mpb->getVarName());
                proxyfileWriter->addData(");\n");
        }
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

        std::vector<ModelProxyBuild*> mpbs = moduleSimEngine->recruitForOpEndCycle();

        proxyfileWriter->addData("////////////////////// standard op\n");
        for (ModelProxyBuild* mpb: mpbs){
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
        proxyfileWriter->addData("///////////////////// memory op\n");
        for (ModelProxyBuild* mpb: mpbs){
            proxyfileWriter->addData(mpb->createMemBlkAssOp());
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
                proxyfileWriter->addData("vcdWriter->addNewVar(");

                //////// sigtype
                VCD_SIG_TYPE  vst = mpb->getSigType();
                if (vst == VST_REG){
                    proxyfileWriter->addData("VST_REG");
                }else if (vst == VST_WIRE){
                    proxyfileWriter->addData("VST_WIRE");
                }else if (vst == VST_INTEGER){
                    proxyfileWriter->addData("VST_INTEGER");
                }

                /////// varname
                proxyfileWriter->addData(",");
                proxyfileWriter->addData("\""+ mpb->getVarName() +"\"");
                proxyfileWriter->addData(",");
                proxyfileWriter->addData("{" +
                    std::to_string(mpb->getSize().start) + "," +
                    std::to_string(mpb->getSize().stop)  + "};\n");
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
                proxyfileWriter->addData("vcdWriter->addNewValue(");

                proxyfileWriter->addData("\""+ mpb->getVarName() +"\"");
                proxyfileWriter->addData(",");
                proxyfileWriter->addData(mpb->getVarName() +
                                    ".genBiStr();\n");
            }
        }

        proxyfileWriter->addData("}\n");
    }


    void ProxyBuildMng::startWritePerfDec(){
        std::vector<ModelProxyBuild*> dayta =
            moduleSimEngine->recruitPerf();

        proxyfileWriter->addData("/////////////////////// perf variable");

        for (ModelProxyBuild* mpb: dayta){
            proxyfileWriter->addData(mpb->createVariable());
        }

        proxyfileWriter->addData("/////////////////////// perf finish initialize");

    }

    void ProxyBuildMng::startWritePerfCol(){
        std::vector<ModelProxyBuild*> dayta =
    moduleSimEngine->recruitPerf();

        proxyfileWriter->addData("void ProxySimEvent::startPerfCol");
        proxyfileWriter->addData("(){\n");


        for (ModelProxyBuild* mpb: dayta){
            proxyfileWriter->addData(mpb->createOp());
        }

        proxyfileWriter->addData("}\n");
    }

    void ProxyBuildMng::startCompile(){
        int result = system(srcBuilderPath.c_str());
        if (result == 0){
            std::cout << "compile successfully\n";
        }else{
            std::cerr << "Compilation failed with error code " << result << std::endl;
        }
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

        return pixieEvent;
    }

    void ProxyBuildMng::unloadProxy(){
        dlclose(_handle);
    }
}
