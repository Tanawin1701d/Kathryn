//
// Created by tanawin on 25/1/2567.
//

#ifndef KATHRYN_SIMINTERFACE_H
#define KATHRYN_SIMINTERFACE_H

#include <condition_variable>
#include <thread>
#include "model/controller/controller.h"
#include "model/simIntf/moduleSimEvent.h"
#include "sim/event/userEvent.h"
#include "sim/controller/simController.h"
#include "sim/event/ctTrigEvent.h"

/////#define sim agent << [&]()

namespace kathryn{

    class SimInterface{
        friend class ConcreteTriggerEvent;
    protected:
        VcdWriter*              _vcdWriter      = nullptr;
        FlowWriter*             _flowWriter     = nullptr;
        ModuleSimEvent*         _ModuleSimEvent = nullptr;
        std::vector<UserEvent*> _UserSimEvents;
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
                              std::string profileFilePath);

        virtual ~SimInterface();

        void simStart();

        /**describe discreate event*/
        virtual void describe(){};

        /**describe concrete event*/
        virtual void describeCon(){};

        /** user it to communicate about simulation trigger*/

    };

    class testItf: public SimInterface{

        int x;

        void describe() override{

            sim{
                x = 55;
                incCycle(5);
                sim{

                };
            };
            incCycle(4);

        }
    };

}

#endif //KATHRYN_SIMINTERFACE_H
