//
// Created by tanawin on 18/1/2567.
//

#ifndef KATHRYN_MODELSIMINTERFACE_H
#define KATHRYN_MODELSIMINTERFACE_H

#include <cassert>
#include <utility>
#include "model/simIntf/modelSimEngine.h"
#include "sim/event/eventBase.h"


namespace kathryn{


    /**
     *
     * process of simulation for kathryn model simulation sequence
     * ------ intiate phase ----------
     * 1. beforePrepareSim(args)    ---- to setVariable before prepare function
     * 2. prepareSim() ----- to process some infomation before simStart
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

    class RtlValItf{
    public:

        virtual ~RtlValItf() = default;

        virtual void setCurValSimStatus()  = 0;
        virtual void setNextValSimStatus() = 0;

        virtual bool     isCurValSim () const= 0;
        virtual bool     isNextValSim() const= 0;

        virtual ValRep&  getCurVal () = 0;
        virtual ValRep&  getNextVal() = 0;

    };

    class SimEngine: public Simulatable, public RtlValItf{

    public:
        virtual ~SimEngine() = default;

    };

    class SimEngineInterface{

    public:
        virtual SimEngine* getSimEngine() = 0;

    };



    /***
     *
     * flow sim interface should not sim/final  Rtl simInterface At all
     * simExitCurCycle for flow sim can be repeatly call in each cycle
     *
     * */

    class FlowSimEngineMaster: public SimEngine{
    protected:
        bool           _isSimulated = false;
        FlowSimEngine* _engine = nullptr;
    public:
        explicit FlowSimEngineMaster(FlowSimEngine* engine):
                SimEngine(),
                _engine(engine){}

        ~FlowSimEngineMaster() override {delete _engine;}

        FlowSimEngine* getSimEngine(){return _engine;}

        /**
         * sim state
         * **/
        bool isCurValSim() const override {
            return _isSimulated;
        }
        void setCurValSimStatus() override{
            _isSimulated = true;
        }
        void unSetSimStatus(){
            _isSimulated = false;
        }

        /**
         * sim state
         * **/
        bool isBlockOrNodeRunning(){
            return _engine->isRunning();
        }
        void setBlockOrNodeRunning(){
            _engine->setRunningStatus();
        }
        void unsetBlockOrNodeRunning(){
            _engine->unsetRunning();
        }
        void incEngine(){
            getSimEngine()->incUsedTime();
            getSimEngine()->setRunningStatus();
        }
        /** initialize data before prepare Sim() is used*/
        virtual void beforePrepareSim(FlowSimEngine::FLOW_Meta_afterMf simMeta){};
        /** start Sim can be invoked multiple times*/
        void prepareSim() override{};
        /** curCycleCollectData*/
        void curCycleCollectData() override{};
        /**simulate next cycle value for current cycle*/
        void simStartNextCycle() override{
            assert(false);
        }

        /** value overload*/
        ValRep&  getCurVal () override {assert(false);}
        ValRep&  getNextVal() override {assert(false);}


    };



    /***
     * module interface
     * */
     class ModuleSimEngine : public SimEngine{

     protected:

     public:
         explicit ModuleSimEngine():
                 SimEngine(){}

         virtual void beforePrepareSim(VcdWriter* vcdWriter, FlowColEle* flowColEle) = 0;

     };

}

#endif //KATHRYN_MODELSIMINTERFACE_H
