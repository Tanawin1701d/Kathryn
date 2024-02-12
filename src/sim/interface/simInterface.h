//
// Created by tanawin on 25/1/2567.
//

#ifndef KATHRYN_SIMINTERFACE_H
#define KATHRYN_SIMINTERFACE_H

#include "model/controller/controller.h"
#include "model/simIntf/moduleSimEvent.h"
#include "sim/event/userEvent.h"
#include "sim/controller/simController.h"

/////#define sim agent << [&]()

namespace kathryn{

    class SimInterface{
    protected:
        VcdWriter*              _vcdWriter      = nullptr;
        FlowWriter*             _flowWriter     = nullptr;
        ModuleSimEvent*         _ModuleSimEvent = nullptr;
        std::vector<UserEvent*> _UserSimEvents;
        CYCLE                   _limitCycle = 0;
        UserEvent               simAgent;/** sim agent base can't change name*/


    public:
        explicit SimInterface(CYCLE limitCycle,
                              std::string vcdFilePath,
                              std::string profileFilePath);

        virtual ~SimInterface();

        void simStart();

        virtual void describe() = 0;



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
