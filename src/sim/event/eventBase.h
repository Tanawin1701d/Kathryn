//
// Created by tanawin on 14/1/2567.
//

#ifndef KATHRYN_EVENTBASE_H
#define KATHRYN_EVENTBASE_H

#include<cassert>

namespace kathryn{

    typedef long long int CYCLE;

    static int SIM_CC_TRIGGER_PRIO_FRONT_CYCLE = 10;
    static int SIM_USER_PRIO  = 10;
    static int SIM_MODEL_PRIO = 9;
    static int SIM_CC_TRIGGER_PRIO_BACK_CYCLE = 8;

    class EventBase{
    protected:
        CYCLE _targetCycle = 0;
        int   _priority = 0;

    public:

        explicit EventBase(CYCLE curCycle, int priority):
                _targetCycle(curCycle),
                _priority(priority)
            {}

        /** it is very crucial to do virtual deconstructor*/
        virtual ~EventBase() = default;
        /**
          * compute value that will be assigned in this cycle
          * */
        virtual void simStartCurCycle() = 0;
        /**
         * collect data from every compute unit
         * ex for reg wire unit will write data to cmd unit
         **/
         virtual void curCycleCollectData() = 0;
        /**
         * a function that used to specify compute unit whether
         * this cycle is finished
         * */
        virtual void simExitCurCycle() = 0;


        /** event base will be schedule by using priority queue
         * the highest priority is the cycle that occur before
         * if cycle number is same _priority will be used to control
         * priority of event queue
         * */
        virtual bool operator < (const EventBase& rhs){
            if ((_targetCycle > rhs._targetCycle) ||
                ((_targetCycle == rhs._targetCycle) && (_priority < rhs._priority))
            ){
                return true;
            }
            return false;
        }

        [[nodiscard]]
        CYCLE getCurCycle() const {return _targetCycle;}
        CYCLE getPriority() const {return _priority;}

        void addNewEvent(EventBase* newEvent);

        virtual bool needToDelete(){
            return true;
        }
    };



}

#endif //KATHRYN_EVENTBASE_H
