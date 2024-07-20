//
// Created by tanawin on 14/1/2567.
//

#ifndef KATHRYN_EVENTBASE_H
#define KATHRYN_EVENTBASE_H

#include<cassert>

namespace kathryn{

    typedef long long int CYCLE;

    static int SIM_CC_TRIGGER_PRIO_FRONT_CYCLE = 10;
    static int SIM_USER_PRIO_FRONT_CYCLE  = 10;
    static int SIM_USER_PRIO_BACK_CYCLE  = 8;
    static int SIM_MODEL_PRIO = 9;
    constexpr int MAX_PROX_CALLBACK_FUNCTION = 25;

    class EventBase{
    protected:
        CYCLE _targetCycle = 0;
        int   _priority = 0;
        //////// when this event is finish it may req callback function
        ///////  limitCycle
        bool  _isLongRangeSim = false;
        CYCLE _amtLimitLongRangeCycle = 1;

    public:

        explicit EventBase(CYCLE curCycle, int priority, bool longRangeSim):
                _targetCycle(curCycle),
                _priority(priority),
                _isLongRangeSim(longRangeSim)
            {}

        /** it is very crucial to do virtual deconstructor*/
        virtual ~EventBase() = default;

        virtual void simStartLongRunCycle() = 0;
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
        * compute value for next cycle
        * */
        virtual void simStartNextCycle() = 0;
        /**
         * a function that used to specify compute unit whether
         * this cycle is finished
         * */

        virtual void simExitCurCycle() = 0;
        /**
         * gen next event if there are
         */
        virtual EventBase* genNextEvent() {return nullptr;}


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

        //void addNewEvent(EventBase* newEvent);

        virtual bool needToDelete(){
            return true;
        }
        virtual int getCallBackNo(int idx) const{
            return -1;
        }
        virtual int getCallBackAmt() const{
            return 0;
        }

        void setLongRangeSim(CYCLE amtCycle){
            assert(_isLongRangeSim);
            _amtLimitLongRangeCycle = amtCycle;
            assert(amtCycle > 0);
        }

        bool isLongRageSim(){return _isLongRangeSim;}

    };



}

#endif //KATHRYN_EVENTBASE_H
