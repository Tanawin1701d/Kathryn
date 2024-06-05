//
// Created by tanawin on 25/1/2567.
//

#ifndef KATHRYN_SIMINTERFACE_H
#define KATHRYN_SIMINTERFACE_H

#include <condition_variable>
#include <thread>
#include "model/controller/controller.h"
#include "modelCompile/proxyEventBase.h"
#include "sim/event/userEvent.h"
#include "sim/controller/simController.h"
#include "sim/event/ctTrigEvent.h"
#include "util/termColor/termColor.h"

/////#define sim agent << [&]()

namespace kathryn{

    class SimInterface{
        friend class ConcreteTriggerEvent;
    protected:
        std::string             SIM_CLIENT_PATH;
        VcdWriter*              _vcdWriter      = nullptr;
        FlowWriter*             _flowWriter     = nullptr;
        ProxySimEventBase*      _modelSimEvent  = nullptr;
        std::vector<UserEvent*> UserSimEvents;
        CYCLE                   _limitCycle = 0;
        UserEvent               simAgent;/** sim agent base can't change name*/

        /**concrete sim*/

        std::unique_ptr<std::thread> conThread;
        ConcreteTriggerEvent*        lastCtTrigger = nullptr;
        CYCLE                        conCurCycleUsed = 2;

        void simStartConSim();
        /**for now we will not support Condition*/
        void conCycleBase(CYCLE startCycle, int priority);
        void conCycle(CYCLE startCycle);
        void conNextCycle(CYCLE amtCycle);
        void conEndCycle();
        //// to initialize system and finalize system
        void describeConWrapper();


    public:
        explicit SimInterface(CYCLE limitCycle,
                              std::string vcdFilePath,
                              std::string profileFilePath,
                              std::string clientSimPath
                              );

        virtual ~SimInterface();

        void simStart();
        /**describe discreate event*/
        virtual void describe(){};
        /**describe concrete event*/
        virtual void describeCon(){};
        /** test value helper*/
        void testAndPrint(const std::string& testName, ValRepBase& simVal, ValRepBase& rhs);
        void testAndPrint(const std::string& testName, ull simVal, ull expect);
        /** build proxy SimEvent*/
        void createModelSimEvent();


    };

}

#endif //KATHRYN_SIMINTERFACE_H
