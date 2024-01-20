//
// Created by tanawin on 18/1/2567.
//

#ifndef KATHRYN_RTLSIMITF_H
#define KATHRYN_RTLSIMITF_H

#include <cassert>
#include "model/simIntf/rtlSimEle.h"
#include "sim/event/eventBase.h"


namespace kathryn{

    class SimInterface : public EventBase{
    private:
        bool isSimYet = false; ///// indicate that current cycle is simulated yet.
    public:
        SimInterface():
        EventBase(/**get current cycle**/)
        {
            ////// assert(engine != nullptr);
            //////// for now we allow engine to be nullptr
        };

        void setSimStatus(){
            isSimYet = true;
        }
        void resetSimStatus(){
            isSimYet = false;
        }
        [[nodiscard]]
        bool isCurCycleSimulated() const{
            return isSimYet;
        }

    };


    class RtlSimInterface : public SimInterface{
    private:
        RtlSimEngine* _engine = nullptr;
    public:

        explicit RtlSimInterface(RtlSimEngine* engine):
        SimInterface(),
        _engine(engine){}

        ~RtlSimInterface() override {delete _engine;}

        void setSimEngine(RtlSimEngine* engine){
            if (_engine != nullptr){
                delete _engine;
            }
            _engine = engine;
        }

        RtlSimEngine* getSimEngine(){return _engine;}



    };

    /***
     *
     * flow sim interface should not sim/final  Rtl simInterface At all
     * simExitCurCycle for flow sim can be repeatly call in each cycle
     *
     * */

    class FlowSimInterface : public SimInterface{
    private:
        FlowSimEngine* _engine = nullptr;
    public:
        explicit FlowSimInterface(FlowSimEngine* engine):
        SimInterface(),
        _engine(engine){}

        ~FlowSimInterface() override {delete _engine;}

        FlowSimEngine* getSimEngine(){return _engine;}

        bool incEngine(bool isStateRunning){

            if (isStateRunning){
                getSimEngine()->incUsedTime();
            }
            getSimEngine()->setRunningStatus(isStateRunning);

        }

        bool isStateSetInCurCycle() {
            assert(isCurCycleSimulated());
            return getSimEngine()->isRunning();
        };

        void resetFlowSimStatus(){
            resetSimStatus();
            assert(_engine != nullptr);
            _engine->unsetRunning();
        };



    };

}

#endif //KATHRYN_RTLSIMITF_H
