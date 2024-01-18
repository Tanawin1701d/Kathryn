//
// Created by tanawin on 18/1/2567.
//

#ifndef KATHRYN_RTLSIMELE_H
#define KATHRYN_RTLSIMELE_H

#include "sim/logicRep/valRep.h"

namespace kathryn {

    class RtlSimEngine{

    public:

        explicit RtlSimEngine();
        virtual ~RtlSimEngine() = default;

        /** get current value from back cycle to
         * compute next cycle
         * */
        virtual ValRep& getValToCalThisCycle() = 0;

    };


    //////////////// sequential sim


    class seqRtlSimEngine : public RtlSimEngine{

    private:
        ValRep backVal;
        ValRep curVal;

    public:

        explicit seqRtlSimEngine(int sz);

        ValRep& getValToCalThisCycle() override;
        ///// setter
        void    setBackVal(ValRep& val){backVal = val;}
        void    setCurVal (ValRep& val){curVal  = val;}
        ///// getter
        ValRep& getBackVal(){return backVal;}
        ValRep& getCurVal (){return curVal; }

    };


    //////////////// comb sim

    class combRtlSimEngine : public RtlSimEngine{

    private:
        ValRep curVal;

    public:
        explicit combRtlSimEngine(int sz);

        ValRep& getValToCalThisCycle() override;

    };

}
#endif //KATHRYN_RTLSIMELE_H
