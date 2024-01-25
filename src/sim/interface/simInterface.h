//
// Created by tanawin on 25/1/2567.
//

#ifndef KATHRYN_SIMINTERFACE_H
#define KATHRYN_SIMINTERFACE_H

#include "model/simIntf/simEvent.h"
#include "sim/event/userEvent.h"
#include "sim/controller/controller.h"
#include "model/controller/controller.h"



namespace kathryn{

    class SimInterface{
    protected:
        ModuleSimEvent*         _ModuleSimEvent;
        std::vector<UserEvent*> _UserSimEvents;
        CYCLE                   _curUserDescCycle = 0;
        /** to receive uservent from user description*/
        class UserSimAgent{
        public:
            SimInterface* _master;
            explicit UserSimAgent(SimInterface* master);
            UserSimAgent& operator << (std::function<void(void)> simBehaviour);
        };


    public:
        explicit SimInterface();

        ~SimInterface();

        void simStart();

        void incCycle(CYCLE inCycle);
        void setCycle(CYCLE stCycle);

        virtual void describe() = 0;



    };

}

#endif //KATHRYN_SIMINTERFACE_H
