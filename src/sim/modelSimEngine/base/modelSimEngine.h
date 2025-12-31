//
// Created by tanawin on 18/1/2567.
//

#ifndef KATHRYN_MODELSIMENGINE_H
#define KATHRYN_MODELSIMENGINE_H

#include <cassert>
#include <utility>
#include "sim/logicRep/valRep.h"

#include "sim/event/eventBase.h"


namespace kathryn{


    /**
     * [legacy]
     *
     * process of simulation for kathryn model simulation sequence
     * ------ intiate phase ----------
     * 1. beforePrepareSim(args)    ---- to setVariable before prepare function
     * 2. prepareSim() ----- to process some infomation before start
     * ------ sim phase(loop)----------
     * 3. simStartCurCycle()    ----- to simulate value only in current cycle
     * 4. simStartNextCycle()   ----- to simulate value only in next cycle
     * 4. curCycleCollectData() ----- to collect data for simulation that time
     * [[unused]]5. simExitCollectData()  ----- to prepare simulation for next cycle
     * */

    class Simulatable{
    public:
        Simulatable() = default;

        virtual ~Simulatable() = default;

        virtual void prepareSim() = 0;
        /**
          * compute value that will be assigned in this cycle
          * but store in buffer place
          * */
        virtual void simStartCurCycle() = 0;

        /**
         * simulate next cycle value from current cycle
         * */
        virtual void simStartNextCycle() = 0;
        /**
         * collect data to stat or vcd file
         * */
        virtual void curCycleCollectData() = 0;
        /**
         * move value from buffer place to actual place
         * we do these because we need to maintain edge trigger
         * to not cascade change value while other rtl block is updating
         * */
        virtual void simExitCurCycle() = 0;
        /**
         * collect data that must be collect when simulation is
         * simmulated
         * **/



    };


}

#endif //KATHRYN_MODELSIMENGINE_H
