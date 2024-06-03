//
// Created by tanawin on 1/6/2024.
//

#include "proxyBuildMng.h"

#include <set>


namespace kathryn{

    ProxyBuildMng::ProxyBuildMng(Module* startModule):
    _startModule(startModule),
    moduleSimEngine(startModule->getSimEngine()),
    fileWriter(nullptr)
    {
        fileWriter = new FileWriterBase(desPath);
        assert(fileWriter != nullptr);
        assert(startModule != nullptr);
    }

    ProxyBuildMng::~ProxyBuildMng(){
        fileWriter->flush();
        delete fileWriter;
    }

    std::vector<ModelProxyBuild*> ProxyBuildMng::doTopologySort(
        std::vector<ModelProxyBuild*>& graph){

        if (graph.empty()){return {};}

        struct MPD_META{
            ModelProxyBuild* mpd = nullptr;
            int nextDepIdx = 0;
        };

        /** initialize variable*/
        std::set<ModelProxyBuild*>    visited;
        std::set<ModelProxyBuild*>    resulted;
        std::set<ModelProxyBuild*>    inGraph; ////// somedep is come from not related sorce
        std::vector<ModelProxyBuild*> result;
        std::stack<MPD_META>          dfs;
        ///////////////////////////////////////////////
        for (ModelProxyBuild* mdp: graph){ ///
            inGraph.insert(mdp);
        }


        for (auto ele: graph){
            assert(!dfs.empty());
            dfs.push({ele, 0});

            while(!dfs.empty()){
                MPD_META& top = dfs.top();

                if (inGraph.find(top.mpd) == inGraph.end()){
                    dfs.pop();
                    continue;
                }

                bool isVisit  = visited.find(top.mpd)  != visited.end();
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


    void ProxyBuildMng::startWriteModelSim(){

        //// create include
        fileWriter->addData("#include \"src/modelCompile/proxyEvent.h\"\n");

        /// create namespace
        ///
        fileWriter->addData("namespace kathryn{\n\n\n\n\n\n");

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


        fileWriter->addData("\n\n\n\n\n\n}\n");

    }

    void ProxyBuildMng::startWriteCreateVariable(){
        assert(moduleSimEngine != nullptr);
        std::vector<ModelProxyBuild*> dayta =
            moduleSimEngine->recruitForCreateVar();

        for (ModelProxyBuild* mpb: dayta){
            fileWriter->addData(mpb->createVariable());
            fileWriter->addData("\n");
        }
    }

    void ProxyBuildMng::startWriteRegisterCallback(){
        std::vector<ModelProxyBuild*> dayta =
            moduleSimEngine->recruitForCreateVar();

        fileWriter->addData("void ProxySimEvent::startRegisterCallBack(){\n");


        for (ModelProxyBuild* mpb: dayta){
                fileWriter->addData("       ");
                fileWriter->addData("registerToCallBack(");
                fileWriter->addData("\"" + mpb->getVarName()+ "\"");
                fileWriter->addData(",");
                fileWriter->addData(mpb->getVarName());
                fileWriter->addData(");\n");
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

        fileWriter->addData("void ProxySimEvent::startVolatileEleSim(){\n");

        std::vector<ModelProxyBuild*> mpbs = moduleSimEngine->recruitForNonVolatileEle();

        fileWriter->addData("////////////////////// standard op\n");
        for (ModelProxyBuild* mpb: mpbs){
            fileWriter->addData(mpb->createOp());
        }
        fileWriter->addData("}\n");

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

        fileWriter->addData("void ProxySimEvent::startNonVolatileEleSim(){\n");

        std::vector<ModelProxyBuild*> mpbs = moduleSimEngine->recruitForNonVolatileEle();

        fileWriter->addData("////////////////////// standard op\n");
        for (ModelProxyBuild* mpb: mpbs){
            fileWriter->addData(mpb->createOp());
        }
        fileWriter->addData("///////////////////// memory op\n");
        for (ModelProxyBuild* mpb: mpbs){
            fileWriter->addData(mpb->createMemBlkAssOp());
        }
        fileWriter->addData("//////////////////// transfer Op\n");

        for (ModelProxyBuild* mpb: mpbs){
            fileWriter->addData(mpb->createMemorizeOp());
        }

        fileWriter->addData("}\n");

    }


    void ProxyBuildMng::startWriteVcdDecVar(bool isUser){
        std::vector<LogicSimEngine*> dayta =
            moduleSimEngine->recruitAllLogicSimEngine();

        fileWriter->addData("void ProxySimEvent::startVcdDecVar");
        fileWriter->addData((isUser? "User": "Internal"));
        fileWriter->addData("(){\n");


        for (LogicSimEngine* mpb: dayta){
            if (mpb->isUserDeclare() == isUser){ ////// the registerable must always put to vcd file
                fileWriter->addData("       ");
                fileWriter->addData("vcdWriter->addNewVar(");

                //////// sigtype
                VCD_SIG_TYPE  vst = mpb->getSigType();
                if (vst == VST_REG){
                    fileWriter->addData("VST_REG");
                }else if (vst == VST_WIRE){
                    fileWriter->addData("VST_WIRE");
                }else if (vst == VST_INTEGER){
                    fileWriter->addData("VST_INTEGER");
                }

                /////// varname
                fileWriter->addData(",");
                fileWriter->addData("\""+ mpb->getVarName() +"\"");
                fileWriter->addData(",");
                fileWriter->addData("{" +
                    std::to_string(mpb->getSize().start) + "," +
                    std::to_string(mpb->getSize().stop)  + "};\n");
            }
        }
        fileWriter->addData("}\n");
    }

    void ProxyBuildMng::startWriteVcdCol(bool isUser){
        std::vector<LogicSimEngine*> dayta =
            moduleSimEngine->recruitAllLogicSimEngine();

        fileWriter->addData("void ProxySimEvent::startVcdCol");
        fileWriter->addData((isUser? "User": "Internal"));
        fileWriter->addData("(){\n");


        for (ModelProxyBuild* mpb: dayta){
            if (mpb->isUserDeclare() == isUser){ ////// the registerable must always put to vcd file
                fileWriter->addData("       ");
                fileWriter->addData("vcdWriter->addNewValue(");

                fileWriter->addData("\""+ mpb->getVarName() +"\"");
                fileWriter->addData(",");
                fileWriter->addData(mpb->getVarName() +
                                    ".genBiStr();\n");
            }
        }

        fileWriter->addData("}\n");
    }


    void ProxyBuildMng::startWritePerfDec(){
        std::vector<ModelProxyBuild*> dayta =
            moduleSimEngine->recruitForCreateVar();

        fileWriter->addData("/////////////////////// perf variable");


        for (ModelProxyBuild* mpb: dayta){
            if (mpb->isFlowBlockIden()){
                fileWriter->addData("ull "+ PERF_PREFIX +mpb->getVarName());
                fileWriter->addData(" = 0;\n");
            }
        }

        fileWriter->addData("/////////////////////// perf finish initialize");

    }

    void ProxyBuildMng::startWritePerfCol(){
        std::vector<ModelProxyBuild*> dayta =
    moduleSimEngine->recruitForCreateVar();

        fileWriter->addData("void ProxySimEvent::startPerfCol");
        fileWriter->addData("(){\n");


        for (ModelProxyBuild* mpb: dayta){
            if (mpb->isFlowBlockIden()){
                fileWriter(PERF_PREFIX + mpb->getVarName());
                fileWriter(" += ");
                fileWriter(mpb->getVarName() + ".getLogicValue();\n");
            }
        }

        fileWriter->addData("}\n");
    }










}
