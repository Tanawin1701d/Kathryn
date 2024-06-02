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

        startWriteCreateVariable();
        startWriteVolatileEleSim();
        startWriteNonVolatileEleSim();

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






}
