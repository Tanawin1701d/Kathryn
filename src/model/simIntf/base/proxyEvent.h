//
// Created by tanawin on 1/6/2024.
//

#ifndef PROXYEVENT_H
#define PROXYEVENT_H

#include <unordered_map>

#include "model/debugger/modelDebugger.h"
#include "params/simParam.h"
#include "sim/logicRep/valRep.h"
#include "sim/event/eventBase.h"
#include "sim/simResWriter/simResWriter.h"

namespace kathryn{


    class ProxySimEvent: public EventBase{
    protected:
        std::string vcdWriteDes;
        std::unordered_map<std::string, ValRepBase*>  callBack; //// for mem block use start point

        VcdWriter* vcdWriter;
        /////// for flow collection we will let flow model handle this instead
        /// we just collect only data that flow block use

    public:
        ////// constructor
        explicit ProxySimEvent();
        ~ProxySimEvent() override;
        ////// sim base
        void simStartCurCycle()    override;
        void curCycleCollectData() override;
        void simStartNextCycle()   override;
        void simExitCurCycle()     override{}

        bool needToDelete()        override{return false;}
        void setVcdFilePath(std::string vcdFilePath);

        void registerToCallBack(const std::string& cbName, ValRepBase* val){
            assert(val != nullptr && (callBack.find(cbName) == callBack.end()));
            callBack.insert({cbName, val});
        }

        ValRepBase* getValRep(std::string globalName){
            mfAssert(callBack.find(globalName) == callBack.end(),
                        "cannot find name at callback"  + globalName);
            return callBack[globalName];
        };
        ////// sim proxy
        void startRegisterCallBack();
        void startVolatileEleSim();
        void startNonVolatileEleSim();
        ///// vcd collector
        void startVcdDecVarUser();
        void startVcdDecVarInternal();
        void startVcdColUser();
        void startVcdColInternal();
        ///// start flowblock CollectData
        void startPerfCol();




    };



}

#endif //PROXYEVENT_H
