//
// Created by tanawin on 18/1/2567.
//

#ifndef KATHRYN_RTLSIMITF_H
#define KATHRYN_RTLSIMITF_H

#include <cassert>
#include "model/simIntf/rtlSimEle.h"
#include "sim/event/eventBase.h"


namespace kathryn{

    class Simulatable{
    private:
        bool isSimYet = false; ///// indicate that current cycle is simulated yet.
    public:
        Simulatable() = default;

        virtual ~Simulatable() = default;

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
        /**
          * compute value that will be assigned in this cycle
          * but store in buffer place
          * */
        virtual void simStartCurCycle() = 0;
        /**
         * move value from buffer place to actual place
         * we do these because we need to maintain edge trigger
         * to not cascade change value while other rtl block is updating
         * */
        virtual void simExitCurCycle() = 0;

    };


    class RtlSimulatable : public Simulatable{
    private:
        RtlSimEngine* _engine = nullptr;
    public:

        explicit RtlSimulatable(RtlSimEngine* engine):
                Simulatable(),
                _engine(engine){}

        ~RtlSimulatable() override {delete _engine;}

        void setSimEngine(RtlSimEngine* engine){
            if (_engine != nullptr){
                delete _engine;
            }
            _engine = engine;
        }

        RtlSimEngine* getSimEngine(){return _engine;}

        /** exit sim can be only invoked single time per cycle*/

        void setToRec() {_engine->setRecordCmd(true);}
        std::vector<ValRep>& getRecData() { return _engine->getRecordData(); }


    };

    /***
     *
     * flow sim interface should not sim/final  Rtl simInterface At all
     * simExitCurCycle for flow sim can be repeatly call in each cycle
     *
     * */

    class FlowSimulatable : public Simulatable{
    private:
        FlowSimEngine* _engine = nullptr;
    public:
        explicit FlowSimulatable(FlowSimEngine* engine):
                Simulatable(),
                _engine(engine){}

        ~FlowSimulatable() override {delete _engine;}

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

        /** start Sim can be invoked multiple times*/
        /** exit sim can be invoked multiple times*/
    };

    /***
     * module interface
     * */
     class ModuleSimInterface : public Simulatable{
     public:
         explicit ModuleSimInterface():
                 Simulatable(){}

         /** start Sim can be invoked multiple times*/
         /** exit sim can be invoked multiple times*/
     };

}

#endif //KATHRYN_RTLSIMITF_H
