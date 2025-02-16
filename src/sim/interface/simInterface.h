//
// Created by tanawin on 25/1/2567.
//

#ifndef KATHRYN_SIMINTERFACE_H
#define KATHRYN_SIMINTERFACE_H

#include <condition_variable>
#include <thread>
#include "sim/modelSimEngine/base/proxyBuildMng.h"
#include "model/controller/controller.h"
#include "sim/event/userEvent.h"
#include "sim/controller/simController.h"
#include "sim/event/ctTrigEvent.h"
#include "sim/modelSimEngine/base/traceEvent.h"
#include "util/termColor/termColor.h"

/////#define sim agent << [&]()

namespace kathryn{




    class SimInterface{
        friend class ConcreteTriggerEvent;
    protected:
        SimProxyBuildMode       _simProxyBuildMode;
        VcdWriter*              _vcdWriter      = nullptr;
        FlowWriter*             _flowWriter     = nullptr;
        ProxySimEventBase*      _modelSimEvent  = nullptr;
        std::vector<UserEvent*> UserSimEvents;
        CYCLE                   _limitCycle = 0;
        UserEvent               simAgent;/** sim agent base can't change name*/
        ProxyBuildMng           _proxyBuildMng;

        //////////////////////////
        /**long range simulation*/
        //////////////////////////
        std::vector<TraceEvent> _traceEvents;
        bool                    _requireLRC       = false;
        CYCLE                   _nextLimitAmtLRC  = INT64_MAX; //// next limit amount long run cyclc
        ///////////////////
        /**concrete sim*/
        ///////////////////
        std::unique_ptr<std::thread> conThread;
        bool                         requireConSim  = true;
        ConcreteTriggerEvent*        lastCtTrigger = nullptr;
        CYCLE                        conCurCycleUsed = 2;

        void simStartConSim();
        /**for now we will not support Condition*/
        void conCycleBase(CYCLE startCycle);
        void conCycle(CYCLE startCycle);
        void conNextCycle(CYCLE amtCycle);
        void conEndCycle();
        //// to initialize system and finalize system
        void describeConWrapper();
        //////////////////////////////////////////////////////
        ///////////////////
        ////**trigger mng*/
        enum SIM_INTERFACE_EVENT{
            EXIT_SIM
        };
        void setNextLimitAmtLRC(CYCLE amtCycle);
        void trig(Operable& opr, std::function<void()> callback);
        void trig(Operable& opr, SIM_INTERFACE_EVENT event);
        void describeModelTriggerWrapper();


    public:
        explicit SimInterface(CYCLE limitCycle,
                              std::string vcdFilePath,
                              std::string profileFilePath,
                              std::string genFileName = "proxySimAutoGen.cpp",
                              SimProxyBuildMode simProxyBuildMode =
                                    SimProxyBuildMode::SPB_GEN |
                                    SimProxyBuildMode::SPB_COMPILE |
                                    SimProxyBuildMode::SPB_RUN,
                              bool requireLRC = false ///// this will enable model sim event start with long range simulator
                              );

        virtual ~SimInterface();

        void simStart();
        /**describe default value*/
        virtual void    describeDef();
        /**describe model trigger value*/
        virtual void    describeModelTrigger(){};
        /**describe discreate event*/
        virtual void describe(){};
        /**describe concrete event*/
        virtual void describeCon(){};

        /** test value helper*/
        void testAndPrint(const std::string& testName, ValRepBase& simVal, ValRepBase& rhs);
        void testAndPrint(const std::string& testName, ull simVal, ull expect);
        /** build proxy SimEvent*/
        void createModelSimEvent();
        void initPerfCol();
        void finalPerfCol();
        /** require con sim*/
        void disableConSim(){requireConSim = false;} ///// it will not lunch con thead simulator
        /** require LRC (this can be invoked before simulation is start only)*/
        void enableLRC(){_requireLRC = true;}

        ProxySimEventBase* getProxySimEvent(){return _modelSimEvent;}




    };

}

#endif //KATHRYN_SIMINTERFACE_H
