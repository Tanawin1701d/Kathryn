//
// Created by tanawin on 1/6/2024.
//

#ifndef MODEL_COMPILE_PROXY_EVENT_BASE_H
#define MODEL_COMPILE_PROXY_EVENT_BASE_H

#include <unordered_map>
#include <utility>
#include "sim/logicRep/valRep.h"
#include "sim/event/eventBase.h"
#include "sim/simResWriter/simResWriter.h"

namespace kathryn{


    class ProxySimEventBase: public EventBase{
    protected:
        VcdWriter* _vcdWriter = nullptr;
        CYCLE curCycle = 0;

        std::unordered_map<std::string, ValRepBase*>  callBack; //// for mem block use start point
        std::unordered_map<std::string, ValRepBase*>  callBackPerf;

        /////// for flow collection we will let flow model handle this instead
        /// we just collect only data that flow block use

    public:
        ////// constructor
        explicit ProxySimEventBase();
        ~ProxySimEventBase() override;
        ////// sim base
        void eventWarmUp();
        void simStartCurCycle()    override;
        void curCycleCollectData() override;
        void simStartNextCycle()   override;
        void simExitCurCycle()     override{}

        bool needToDelete()        override{return false;}

        void setVcdWriter(VcdWriter* vcdWriter){_vcdWriter = vcdWriter;}
        void registerToCallBack(const std::string& cbName, ValRepBase* val){
            assert(val != nullptr && (callBack.find(cbName) == callBack.end()));
            callBack.insert({cbName, val});
        }

        static ValRepBase* getValRepBase(std::unordered_map<std::string, ValRepBase*>& callBackSorce,
                                  std::string globalName){
            assert(callBackSorce.find(globalName) != callBackSorce.end());
            return callBackSorce[globalName];
        }
        ValRepBase*        getValRep    (std::string globalName){return getValRepBase(callBack    , std::move(globalName));}
        ValRepBase*        getValRepPerf(std::string globalName){return getValRepBase(callBackPerf, std::move(globalName));}
        ////// sim proxy
        virtual void startRegisterCallBack()  = 0;
        virtual void startVolatileEleSim()    = 0;
        virtual void startNonVolatileEleSim() = 0;
        ///// vcd collector
        virtual void startVcdDecVarUser()     = 0;
        virtual void startVcdDecVarInternal() = 0;
        virtual void startVcdColUser()        = 0;
        virtual void startVcdColInternal()    = 0;
        ///// start flowblock CollectData
        virtual void startPerfCol()           = 0;

    };



}

#endif //MODEL_COMPILE_PROXY_EVENT_BASE_H
