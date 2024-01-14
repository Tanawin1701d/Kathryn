//
// Created by tanawin on 14/1/2567.
//

#ifndef KATHRYN_EVENTBASE_H
#define KATHRYN_EVENTBASE_H

namespace kathryn{

    typedef unsigned long long int CYCLE;

    class EventBase{
    private:
        CYCLE _curCycle = 0;
        int   _priority = 0;

    public:

        explicit EventBase(CYCLE curCycle): _curCycle(curCycle){}

        virtual void simCurCycle() = 0;

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

    };



}

#endif //KATHRYN_EVENTBASE_H
