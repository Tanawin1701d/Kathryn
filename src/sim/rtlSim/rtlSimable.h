//
// Created by tanawin on 14/1/2567.
//

#ifndef KATHRYN_RTLSIMABLE_H
#define KATHRYN_RTLSIMABLE_H

namespace kathryn{

    class RtlSimulatable{

    public:
        /**
         * compute value that will be assigned in this cycle
         * but store in buffer place
         * */
        virtual void simCurCycle() = 0;
        /**
         * move value from buffer place to actual place
         *
         * we do these because we need to maintain edge trigger
         * to not cascade change value while other rtl block is updating
         * */
        virtual void finalizeCurCycle() = 0;

    };

}

#endif //KATHRYN_RTLSIMABLE_H
