//
// Created by tanawin on 18/1/2567.
//

#ifndef KATHRYN_RTLSIMITF_H
#define KATHRYN_RTLSIMITF_H

#include <cassert>
#include "model/simIntf/rtlSimEle.h"
#include "sim/event/eventBase.h"


namespace kathryn{

    class SimInterface : EventBase{
    private:
        bool isSimYet = false; ///// indicate that current cycle is simulated yet.
        RtlSimEngine* _engine = nullptr;
    public:
        SimInterface(RtlSimEngine* engine):
        EventBase(/**get current cycle**/),
        _engine(engine)
        {
            ////// assert(engine != nullptr);
            //////// for now we allow engine to be nullptr
        };

        void setSimEngine(RtlSimEngine* engine){
            if (_engine != nullptr){
                delete _engine;
            }
            _engine = engine;
        }

        RtlSimEngine* getEngine(){return _engine;}

        void setSimStatus(){
            isSimYet = true;
        }
        void resetSimStatus(){
            isSimYet = false;
        }
        bool isCurCycleSimulated() const{
            return isSimYet;
        }

    };

}

#endif //KATHRYN_RTLSIMITF_H
