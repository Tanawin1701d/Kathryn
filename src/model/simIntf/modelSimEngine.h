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
        bool               _recMode      = false; //// is this engine recordable
        bool               _simForNext   = false;
        bool               _isSimMetaSet = false;
        VCD_SIG_TYPE       _sigType = VST_DUMMY;
        RTL_Meta_afterMf   _simMeta;
        /** sz*/
        int                _sz;
        /** check that current and next cycle is simulate*/
        bool               _isCurValSim;
        bool               _isNextValSim;
        /** value that system is simulated */
        ValRep             _curVal;
        ValRep             _nextVal;
        /** idea we will use time array to store history of cycle
         *  but for now we use cur cycle and next cycle
         * */
    public:
        explicit RtlSimEngine(int sz);
        explicit RtlSimEngine(int sz, VCD_SIG_TYPE sigType, bool simForNext);

        virtual ~RtlSimEngine() = default;

        ////// setter
        void setSimForNext(bool v) {_simForNext = v;}
        void setSimMeta  (RTL_Meta_afterMf& val) { _simMeta = val; _isSimMetaSet = true;}
        void setCurValSimStatus()  {_isCurValSim  = true;}
        void setNextValSimStatus() {_isNextValSim = true;}

        ///// getter
        bool              isSimForNext() const{return _simForNext;}
        bool              isCurValSim () const{return _isCurValSim;}
        bool              isNextValSim() const{return _isNextValSim;}
        ValRep&           getCurVal () { return _curVal; }
        ValRep&           getNextVal() {return _nextVal; }
        RTL_Meta_afterMf& getSimMeta() {return _simMeta;}
        ///// setRecord
        ///// step
        void iterate() {
            _isCurValSim  = _isNextValSim;
            _isNextValSim = false;
            _curVal       = _nextVal;
            _nextVal      = ValRep(_sz);
        };
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
        bool _isSimMetaSet    = false;
    public:
        struct FLOW_Meta_afterMf{

            ///std::string      _recName = "UNDEFINED_SIGNAL_RECORD_NAME";
            FlowColEle*      _writer  = nullptr;
        };

        FLOW_Meta_afterMf _meta;

        explicit FlowSimEngine() = default;

        void incUsedTime();
        int&  getAmtUsed(){return amtUsed;}

        void setSimMeta(FLOW_Meta_afterMf meta){
            _isSimMetaSet = true;
            _meta = std::move(meta);
        }

        void setRunningStatus  (){isStateRunningIn = true;}
        void unsetRunning()      {isStateRunningIn = false;}
        bool isRunning   ()const {return isStateRunningIn; }
    };
}
#endif //KATHRYN_MODELSIMENGINE_H
