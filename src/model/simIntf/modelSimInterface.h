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


    class RtlSimulatable : public Simulatable{
    private:
        RtlSimEngine* _engine  = nullptr;
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

        /**specific set element before prepare sim*/
        virtual void beforePrepareSim(RtlSimEngine::RTL_Meta_afterMf simMeta){
            _engine->setSimMeta(simMeta);
        }

        /** before sim controller start prepare the system*/
        virtual void prepareSim() override{
            _engine->declareSimVar();
        }

        /**simulate next cycle value for current cycle*/
        void simStartNextCycle() override{
            assert(false);
        }

        /****/
        void curCycleCollectData() override{
            _engine->tryWriteValue();
        }

        /**we sure that it can be invoked only one*/
        void simExitCurCycle() override{
            assert(_engine->isCurValSim());
            _engine->iterate();
        }

    };

    /***
     *
     * flow sim interface should not sim/final  Rtl simInterface At all
     * simExitCurCycle for flow sim can be repeatly call in each cycle
     *
     * */

    class FlowSimulatable : public Simulatable{
    protected:
        bool           _isSimulated = false;
        FlowSimEngine* _engine = nullptr;
    public:
        explicit FlowSimulatable(FlowSimEngine* engine):
                Simulatable(),
                _engine(engine){}

        ~FlowSimulatable() override {delete _engine;}

        FlowSimEngine* getSimEngine(){return _engine;}

        /**
         * sim state
         * **/
        bool isCurCycleSimulated() const {
            return _isSimulated;
        }
        void setSimStatus(){
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
        /**simulate next cycle value for current cycle*/
        void simStartNextCycle() override{
            assert(false);
        }
        /** start Sim can be invoked multiple times*/
        void prepareSim() override{};

        void curCycleCollectData() override{};
        /** exit sim can be invoked multiple times*/

    };

    /***
     * module interface
     * */
     class ModuleSimInterface : public Simulatable{

     public:
         explicit ModuleSimInterface():
                 Simulatable(){}

         virtual void beforePrepareSim(VcdWriter* vcdWriter, FlowColEle* flowColEle) = 0;

         /**simulate next cycle value for current cycle*/
         void simStartNextCycle() override{
             assert(false);
         }

     };

}

#endif //KATHRYN_MODELSIMINTERFACE_H
