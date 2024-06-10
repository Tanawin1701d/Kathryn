//
// Created by tanawin on 1/6/2024.
//

#ifndef MODEL_COMPILE_PROXY_EVENT_BASE_H
#define MODEL_COMPILE_PROXY_EVENT_BASE_H

#include <unordered_map>
#include <utility>

#include "params/simParamType.h"
#include "sim/logicRep/valRep.h"
#include "sim/event/eventBase.h"
#include "sim/simResWriter/simResWriter.h"

namespace kathryn{


    class ProxySimEventBase: public EventBase{
    protected:
        const std::string   CLK_SIGNAL  = "CLK";
        ValRep<1>           clkSignal = 0;
        MODULE_VCD_REC_POL  VCD_REC_POL = MDE_REC_SKIP;
        VcdWriter*          _vcdWriter = nullptr;

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
        EventBase* genNextEvent()        override;

        bool needToDelete()        override{return false;}

        int x [5];

        void setVcdWritePol    (MODULE_VCD_REC_POL vcd_rec_pol){VCD_REC_POL = vcd_rec_pol;}
        void setVcdWriter      (VcdWriter*         vcdWriter  ){_vcdWriter   = vcdWriter;  }
        void registerToCallBack(const std::string& cbName, ValRepBase* val){
            assert(val != nullptr && (callBack.find(cbName) == callBack.end()));
            callBack.insert({cbName, val});
        }
        void registerToCallBack(const std::string& cbName, ValRepBase& val){
            registerToCallBack(cbName, &val);
        }
        void registerToCallBackPerf(const std::string& cbName, ValRepBase* val){
            assert(val != nullptr && (callBackPerf.find(cbName) == callBackPerf.end()));
            callBackPerf.insert({cbName, val});
        }
        void registerToCallBackPerf(const std::string& cbName, ValRepBase& val){
            registerToCallBackPerf(cbName, &val);
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
