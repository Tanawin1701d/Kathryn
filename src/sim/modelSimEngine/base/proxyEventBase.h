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
        ull                 clkSignal   = 0;
        MODULE_VCD_REC_POL  VCD_REC_POL = MDE_REC_SKIP;
        VcdWriter*          _vcdWriter = nullptr;

        std::unordered_map<std::string, uint8_t*>  callBack8; //// for mem block use start point
        std::unordered_map<std::string, uint16_t*> callBack16; //// for mem block use start point
        std::unordered_map<std::string, uint32_t*> callBack32; //// for mem block use start point
        std::unordered_map<std::string, uint64_t*> callBack64; //// for mem block use start point

        std::unordered_map<std::string, uint64_t*> callBackPerf;
        std::unordered_map<std::string, uint8_t*> callBackPerfCurbit;

        CYCLE amtLRSim = 0; ///// amount of long run in each [simStartLongRunCycle]

        /////// for flow collection we will let flow model handle this instead
        /// we just collect only data that flow block use

    public:
        ////// constructor
        explicit ProxySimEventBase();
        ~ProxySimEventBase() override;
        ////// sim base
        void eventWarmUp();
        void simStartLongRunCycle() override;
        void simStartCurCycle()     override;
        void curCycleCollectData()  override;
        void simStartNextCycle()    override;
        void simExitCurCycle()      override{}
        EventBase* genNextEvent()   override;

        bool needToDelete()        override{return false;}

        void setLongRunType    (bool isLongRun)                {_isLongRangeSim = isLongRun;}
        void setVcdWritePol    (MODULE_VCD_REC_POL vcd_rec_pol){VCD_REC_POL = vcd_rec_pol;}
        void setVcdWriter      (VcdWriter*         vcdWriter  ){_vcdWriter   = vcdWriter;  }
        ///////// register callback 8
        void registerToCallBack(const std::string& cbName, uint8_t& val){
            registerToCallBack(cbName, &val);
        }
        void registerToCallBack(const std::string& cbName, uint8_t* val){
            assert(callBack8.find(cbName) == callBack8.end());
            callBack8.insert({cbName, val});
        }
        ///////// register callback 16
        void registerToCallBack(const std::string& cbName, uint16_t& val){
            registerToCallBack(cbName, &val);
        }
        void registerToCallBack(const std::string& cbName, uint16_t* val){
            assert(callBack16.find(cbName) == callBack16.end());
            callBack16.insert({cbName, val});
        }
        ///////// register callback 32
        void registerToCallBack(const std::string& cbName, uint32_t& val){
            registerToCallBack(cbName, &val);
        }
        void registerToCallBack(const std::string& cbName, uint32_t* val){
            assert(callBack32.find(cbName) == callBack32.end());
            callBack32.insert({cbName, val});
        }
        ///////// register callback 64
        void registerToCallBack(const std::string& cbName, uint64_t& val){
            registerToCallBack(cbName, &val);
        }
        void registerToCallBack(const std::string& cbName, uint64_t* val){
            assert(callBack64.find(cbName) == callBack64.end());
            callBack64.insert({cbName, val});
        }

        void registerToCallBackPerf(const std::string& cbName, uint64_t& val){
            assert(callBackPerf.find(cbName) == callBackPerf.end());
            callBackPerf.insert({cbName, &val});
        }

        void registerToCallBackPerf(const std::string& cbName, uint8_t& val){
            assert(callBackPerfCurbit.find(cbName) == callBackPerfCurbit.end());
            callBackPerfCurbit.insert({cbName, &val});
        }

        ValRepBase getVal    (const std::string& globalName){

            if (callBack8.find(globalName) != callBack8.end()){
                return {sizeof(uint8_t), callBack8.find(globalName)->second};
            }
            if (callBack16.find(globalName) != callBack16.end()){
                return {sizeof(uint16_t), callBack16.find(globalName)->second};
            }
            if (callBack32.find(globalName) != callBack32.end()){
                return {sizeof(uint32_t), callBack32.find(globalName)->second};
            }
            if (callBack64.find(globalName) != callBack64.end()){
                return {sizeof(uint64_t), callBack64.find(globalName)->second};
            }
            assert(false);

        }

        ValRepBase getValPerf(const std::string& globalName){
            if (callBackPerf.find(globalName) != callBackPerf.end()){
                return {sizeof(uint64_t), callBackPerf.find(globalName)->second};
            }
            if (callBackPerfCurbit.find(globalName) != callBackPerfCurbit.end()){
                return {sizeof(uint8_t), callBackPerfCurbit.find(globalName)->second};
            }
            assert(false);
        }

        CYCLE getAmtLRsim() const{return amtLRSim;}
        ////// sim proxy
        virtual void startRegisterCallBack()  = 0;
        virtual void startMainOpEleSim()      = 0;
        virtual void startFinalizeEleSim()    = 0;
        ///// vcd collector
        virtual void startVcdDecVarUser()     = 0;
        virtual void startVcdDecVarInternal() = 0;
        virtual void startVcdColUser()        = 0;
        virtual void startVcdColInternal()    = 0;
        ///// start flowblock CollectData
        virtual void startPerfCol()           = 0;

        /////////////////////////////////////////
        ////// start sim optimize ///////////////
        /////////////////////////////////////////

        virtual CYCLE mainSim() = 0;

    };



}

#endif //MODEL_COMPILE_PROXY_EVENT_BASE_H
