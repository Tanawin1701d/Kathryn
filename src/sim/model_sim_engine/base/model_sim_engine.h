//
// Created by tanawin on 18/1/2567.
//

#ifndef KATHRYN_MODELSIMENGINE_H
#define KATHRYN_MODELSIMENGINE_H

#include "cassert"
#include "utility"
#include "sim/logic_rep/val_rep.h"

#include "sim/event/event_base.h"


namespace kathryn{


    /**
     * [legacy]
     *
     * process of simulation for kathryn model simulation sequence
     * ------ intiate phase ----------
     * 1. before_prepare_sim(args)    ---- to set_variable before prepare function
     * 2. prepare_sim() ----- to process some infomation before start
     * ------ sim phase(loop)----------
     * 3. sim_start_cur_cycle()    ----- to simulate value only in current cycle
     * 4. sim_start_next_cycle()   ----- to simulate value only in next cycle
     * 4. cur_cycle_collect_data() ----- to collect data for simulation that time
     * [[unused]]5. sim_exit_collect_data()  ----- to prepare simulation for next cycle
     * */

    class Simulatable{
    public:
        Simulatable() = default;

        virtual ~Simulatable() = default;

        virtual void prepare_sim() = 0;
        /**
          * compute value that will be assigned in this cycle
          * but store in buffer place
          * */
        virtual void sim_start_cur_cycle() = 0;

        /**
         * simulate next cycle value from current cycle
         * */
        virtual void sim_start_next_cycle() = 0;
        /**
         * collect data to stat or vcd file
         * */
        virtual void cur_cycle_collect_data() = 0;
        /**
         * move value from buffer place to actual place
         * we do these because we need to maintain edge trigger
         * to not cascade change value while other rtl block is updating
         * */
        virtual void sim_exit_cur_cycle() = 0;
        /**
         * collect data that must be collect when simulation is
         * simmulated
         * **/



    };


}

#endif //KATHRYN_MODELSIMENGINE_H
