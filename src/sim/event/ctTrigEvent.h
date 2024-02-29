//
// Created by tanawin on 29/2/2567.
//

#ifndef KATHRYN_CTTRIGEVENT_H
#define KATHRYN_CTTRIGEVENT_H

#include <condition_variable>
#include <functional>
#include "eventBase.h"
#include "sim/logicRep/valRep.h"

namespace kathryn{

    class SimInterface;
    class ConcreteTriggerEvent: EventBase{

        friend class SimInterface;
        struct SerializeEvent{
            std::mutex  mtx;
            std::condition_variable conVar;
            bool isProcessed = false;

            void notify(){
                conVar.notify_all();
                mtx.lock();
                isProcessed = true;
                mtx.unlock();
            }

            void wait(){
                std::unique_lock<std::mutex> locker(mtx);
                conVar.wait(locker, [&](){ return isProcessed;});
            }

        };

    private:

        SimInterface* _simInterfaceMaster = nullptr;
        std::function<bool()>& _conditionTrigger;

        SerializeEvent startEvent;
        SerializeEvent finishEvent;

    public:
        explicit ConcreteTriggerEvent(CYCLE targetCycle,
                                      SimInterface* simInterfaceMaster,
                                      std::function<bool()> conditionTrigger
                                      );

        void simStartCurCycle() override;

        void curCycleCollectData() override{};

        void simExitCurCycle() override{};

        SerializeEvent& getStartSerializer(){
            return startEvent;
        }

        SerializeEvent& getFinishSerializer(){
            return finishEvent;
        }


    };

}

#endif //KATHRYN_CTTRIGEVENT_H
