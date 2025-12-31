//
// Created by tanawin on 29/2/2567.
//

#ifndef KATHRYN_CTTRIGEVENT_H
#define KATHRYN_CTTRIGEVENT_H

#include <condition_variable>
#include <functional>
#include <iostream>
#include "eventBase.h"
#include "sim/logicRep/valRep.h"

namespace kathryn{

    class SimInterface;
    class ConcreteTriggerEvent: EventBase{

        friend class SimInterface;
        struct SerializeEvent{
            std::mutex  mtx;
            std::condition_variable conVar;
            CYCLE  lastCycleNotified =  -1;


            void notify(CYCLE yourCycle){
                mtx.lock();
                if (yourCycle >= lastCycleNotified){
                    lastCycleNotified = yourCycle;
                }
                mtx.unlock();
                conVar.notify_all();

            }

            void wait(CYCLE yourCycle){

                std::unique_lock<std::mutex> locker(mtx);
                if (yourCycle <= lastCycleNotified){
                    ////// other is ahead of us
                    locker.unlock();
                    return;
                }

                conVar.wait(locker, [&](){ /****std::cout << isProcessed << std::endl;****/
                return yourCycle <= lastCycleNotified;});
                mtx.unlock();

            }


        };

    private:

        SimInterface* _simInterfaceMaster = nullptr;
        std::function<bool()>& _conditionTrigger;


        SerializeEvent startSimCurEvent;
        SerializeEvent finishSimCurEvent;
        SerializeEvent startEndCycleEvent;
        SerializeEvent finishEndCycleEvent;


        CYCLE nextCycle = -1;
        bool  stop       = false;

    public:
        explicit ConcreteTriggerEvent(CYCLE targetCycle,
                                      SimInterface* simInterfaceMaster,
                                      std::function<bool()> conditionTrigger,
                                      int prority
                                      );

        void simStartLongRunCycle() override{assert(false);}

        void simStartCurCycleNeg()    override;
        void simStartCurCyclePos()    override{}

        void curCycleCollectDataNeg() override{}
        void curCycleCollectDataPos() override{}

        void simStartNextCycleNeg()   override{}
        void simStartNextCyclePos()   override;


        void simExitCurCycle() override{};

        void setFutureCycle(CYCLE futureCycle){nextCycle = futureCycle;}

        EventBase* genNextEvent() override;

        void markStop(){stop = 1;}

        bool needToDelete() override{ return stop;}


    };

}

#endif //KATHRYN_CTTRIGEVENT_H
