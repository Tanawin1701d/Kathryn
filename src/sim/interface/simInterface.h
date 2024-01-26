//
// Created by tanawin on 25/1/2567.
//

#ifndef KATHRYN_SIMINTERFACE_H
#define KATHRYN_SIMINTERFACE_H

#include "model/controller/controller.h"
#include "model/simIntf/moduleSimEvent.h"
#include "sim/event/userEvent.h"
#include "sim/controller/simController.h"

#define sim agent << [&]()

namespace kathryn{

    class SimInterface{
    protected:
        ModuleSimEvent*         _ModuleSimEvent;
        std::vector<UserEvent*> _UserSimEvents;
        CYCLE                   _curUserDescCycle = 0;
        CYCLE                   _limitCycle = 0;
        /** to receive uservent from user description*/
        class UserSimAgent{
        public:
            SimInterface* _master;
            explicit UserSimAgent(SimInterface* master);
            UserSimAgent& operator << (std::function<void(void)> simBehaviour);
        };
        UserSimAgent agent = UserSimAgent(this);

    public:
        explicit SimInterface(CYCLE limitCycle);

        ~SimInterface();

        void simStart();

        void incCycle(CYCLE inCycle);
        void setCycle(CYCLE stCycle);

        virtual void describe() = 0;



    };

//    class testItf: public SimInterface{
//
//        int x;
//
//        void describe() override{
//
//            sim{
//                x = 5;
//            };
//            incCycle(5)
//
//        }
//    };

}

#endif //KATHRYN_SIMINTERFACE_H
