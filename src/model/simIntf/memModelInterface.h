//
// Created by tanawin on 19/2/2567.
//

#ifndef KATHRYN_MEMMODELINTERFACE_H
#define KATHRYN_MEMMODELINTERFACE_H


#include "modelSimInterface.h"

namespace kathryn{


    class MemSimulatable : public Simulatable{
    private:
        /** we will not use sim engine anymore*/
        ValRep* memBlk = nullptr;

    public:
        MemSimulatable();

        /**prepare memory block to be simulate*/
        void prepareSim() override;

        /**simulation*/

        void simStartCurCycle() override;
        /**
         * sim for thing in next cycle, typically write signal
         * */
        void simStartNextCycle() override;
        /**
         * cur cycle collect data
         * */
        void curCycleCollectData() override {assert(false);};

        void simExitCurCycle() override;



    };




}

#endif //KATHRYN_MEMMODELINTERFACE_H
