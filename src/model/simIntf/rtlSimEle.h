//
// Created by tanawin on 18/1/2567.
//

#ifndef KATHRYN_RTLSIMELE_H
#define KATHRYN_RTLSIMELE_H

#include <vector>
#include "sim/logicRep/valRep.h"

namespace kathryn {

    class RtlSimEngine {
    private:
        bool   _recCmd = false; /** do not record data in simulation*/
        std::vector<ValRep> _recData;
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

        ///// setRecord
        void setRecordCmd(bool recCmd){ _recCmd = recCmd;}
        bool getRecordCmd() const {return _recCmd;}
        std::vector<ValRep>& getRecordData(){return _recData;}

        ///// step
        void iterate() {
            if (getRecordCmd())
                _recData.push_back(curVal);
            backVal = curVal;

        };

        //////////////// sequential sim
    };

    class FlowSimEngine{
    private:


        int amtUsed = 0;
        bool isStateRunningIn = false; /// check that are there
                                       /// state is running in this block
    public:

        explicit FlowSimEngine() = default;

        void incUsedTime();
        int&  getAmtUsed(){return amtUsed;}

        void setRunningStatus  ( bool status){isStateRunningIn = status;}
        void unsetRunning()      {isStateRunningIn = false;}
        bool isRunning   ()const {return isStateRunningIn; }
    };
}
#endif //KATHRYN_RTLSIMELE_H
