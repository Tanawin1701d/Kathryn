//
// Created by tanawin on 15/1/2567.
//

#ifndef KATHRYN_SIMCONTROLLER_H
#define KATHRYN_SIMCONTROLLER_H

#include<iostream>
#include<mutex>

#include "model/simIntf/base/proxyEvent.h"
#include "sim/event/eventBase.h"
#include "sim/event/eventQ.h"
#include "abstract/mainControlable.h"

namespace kathryn{

    class ProxySimEvent;
    class Wire;
    class SimController: public MainControlable{
    private:
        std::mutex _rsMtx;
        CYCLE      _limitCycle =  1;
        CYCLE      _curCycle   = -1;
        EventQ     eventQ;
        ProxySimEvent proxySimEvent;

        void collectData();

    public:

        explicit SimController();
        void  setProxySimVcdFile(std::string vcdFilePath);
        void  start() override;
        void  reset() override;
        void  clean() override;
        void  addEvent(EventBase* event);
        void  saveData();
        void  setLimitCycle(CYCLE lmtCycle){_limitCycle = lmtCycle;}
        CYCLE getCurCycle();
        ProxySimEvent* getProxySimEvent();
        void  lock();
        void  unlock();

    };

    SimController* getSimController();

}

#endif //KATHRYN_SIMCONTROLLER_H
