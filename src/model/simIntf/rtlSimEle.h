//
// Created by tanawin on 18/1/2567.
//

#ifndef KATHRYN_RTLSIMELE_H
#define KATHRYN_RTLSIMELE_H

#include "sim/logicRep/valRep.h"

namespace kathryn {

    class RtlSimEngine {

    private:
        ValRep backVal;
        ValRep curVal;
        /** idea we will use time array to store history of cycle
         *  but for now we use cur cycle
         * */
    public:

        explicit RtlSimEngine(int sz);

        virtual ~RtlSimEngine() = default;

        ////// setter
        void setBackVal(ValRep &val) { backVal = val; }

        void setCurVal(ValRep &val) { curVal = val; }

        ///// getter
        ValRep &getBackVal() { return backVal; }

        ValRep &getCurVal() { return curVal; }

        ///// step
        void iterate() {
            backVal = curVal;
        };

        //////////////// sequential sim


    };
}
#endif //KATHRYN_RTLSIMELE_H
