//
// Created by tanawin on 18/1/2567.
//

#ifndef KATHRYN_MODELSIMENGINE_H
#define KATHRYN_MODELSIMENGINE_H

#include <utility>
#include <vector>
#include "sim/logicRep/valRep.h"
#include "sim/simResWriter/simResWriter.h"
#include "model/hwComponent/abstract/Slice.h"

namespace kathryn {



    class RtlSimEngine {
    public:
        struct RTL_Meta_afterMf{
            //// data that require to init after model formation
            /// but before sim formation
            bool         _recCmd  = false;
            std::string  _recName = "UNDEFINED_SIGNAL_RECORD_NAME";
            VcdWriter*   _writer  = nullptr;
        };

    private:
        bool               _isSimMetaSet = false;
        VCD_SIG_TYPE       _sigType = VST_DUMMY;
        RTL_Meta_afterMf   _simMeta;
        ValRep             _backVal;
        ValRep             _curVal;
        /** idea we will use time array to store history of cycle
         *  but for now we use cur cycle
         * */
    public:

        explicit RtlSimEngine(int sz, VCD_SIG_TYPE sigType);

        virtual ~RtlSimEngine() = default;

        ////// setter
        void setBackVal  (ValRep& val)           { _backVal = val;}
        void setCurVal   (ValRep& val)           { _curVal  = val;}
        void setSimMeta  (RTL_Meta_afterMf& val) { _simMeta = val; _isSimMetaSet = true;}
        ///// getter
        ValRep&           getBackVal() { return _backVal; }
        ValRep&           getCurVal () { return _curVal; }
        RTL_Meta_afterMf& getSimMeta() {return _simMeta;}
        ///// setRecord
        ///// step
        void iterate() { _backVal = _curVal;};
        /** data that must be set before simulation*/
        void declareSimVar();
        void tryWriteValue();

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
#endif //KATHRYN_MODELSIMENGINE_H
