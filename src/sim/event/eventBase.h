//
// Created by tanawin on 14/1/2567.
//

#ifndef KATHRYN_EVENTBASE_H
#define KATHRYN_EVENTBASE_H

#include<cassert>

namespace kathryn{

    typedef unsigned long long int CYCLE;

    int SIM_USER_PRIO = 9;
    int SIM_MODEL_PRIO = 8;

    class EventBase{
    private:
        CYCLE _curCycle = 0;
        int   _priority = 0;

    public:

        explicit EventBase(CYCLE curCycle, int priority):
            _curCycle(curCycle),
            _priority(priority)
            {}

        /** it is very crucial to do virtual deconstructor*/
        virtual ~EventBase() = default;
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

        /** event base will be schedule by using priority queue
         * the highest priority is the cycle that occur before
         * if cycle number is same _priority will be used to control
         * priority of event queue
         * */
        virtual bool operator < (const EventBase& rhs){
            if (  (_curCycle  > rhs._curCycle) ||
                 ((_curCycle == rhs._curCycle) && (_priority < rhs._priority))
            ){
                return true;
            }
            return false;
        }

        [[nodiscard]]
        CYCLE getCurCycle() const {return _curCycle;}

        void addNewEvent(EventBase* newEvent);

        virtual bool needToDelete(){
            return true;
        }
    };



}

#endif //KATHRYN_EVENTBASE_H
