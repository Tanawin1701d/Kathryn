//
// Created by tanawin on 14/1/2567.
//

#ifndef KATHRYN_EVENTBASE_H
#define KATHRYN_EVENTBASE_H

#include "cassert"

namespace kathryn{

    typedef long long int CYCLE;

    constexpr int SIM_CC_TRIGGER_PRIO_FRONT_CYCLE = 10;
    constexpr int SIM_USER_PRIO_FRONT_CYCLE  = 10;
    constexpr int SIM_MODEL_PRIO = 9;
    constexpr int SIM_USER_PRIO_BACK_CYCLE  = 8;
    constexpr int MAX_PROX_CALLBACK_FUNCTION = 25;

    class EventBase{
    protected:
        CYCLE _targetCycle = 0;
        int   _priority = 0;
        //////// when this event is finish it may req callback function
        ///////  limit_cycle
        bool  _isLongRangeSim = false;
        CYCLE _amtLimitLongRangeCycle = 1;

    public:

        explicit EventBase(CYCLE cur_cycle, int priority, bool long_range_sim):
                _targetCycle(cur_cycle),
                _priority(priority),
                _isLongRangeSim(long_range_sim)
            {}

        /** it is very crucial to do virtual deconstructor*/
        virtual ~EventBase() = default;

        virtual void sim_start_long_run_cycle() = 0;

        /**
         *
         * |-neg_edge_zone----|-posedge_zone-|
         * ----------------              ----------
         *                |              |
         *                |              |
         *                |--------------|
         */

        /**
          * compute value that will be assigned in this cycle
          * */
        virtual void sim_start_cur_cycle_neg() = 0;
        virtual void sim_start_cur_cycle_pos() = 0;

        /**
         * collect data from every compute unit
         * ex for reg wire unit will write data to cmd unit
         **/
         virtual void cur_cycle_collect_data_neg() = 0;
         virtual void cur_cycle_collect_data_pos() = 0;
        /**
        * compute value for next cycle
        * */
        virtual void sim_start_next_cycle_neg() = 0;
        virtual void sim_start_next_cycle_pos() = 0;
        /**
         * a function that used to specify compute unit whether
         * this cycle is finished
         * */

        virtual void sim_exit_cur_cycle() = 0;
        /**
         * gen next event if there are
         */
        virtual EventBase* gen_next_event() {return nullptr;}


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
        CYCLE get_cur_cycle() const {return _targetCycle;}
        CYCLE get_priority() const {return _priority;}

        //void add_new_event(EventBase* new_event);

        virtual bool need_to_delete(){
            return true;
        }
        [[nodiscard]] virtual int get_call_back_no(int idx) const{
            return -1;
        }
        [[nodiscard]] virtual int get_call_back_amt() const{
            return 0;
        }

        void set_long_range_sim(CYCLE amt_cycle){
            assert(_isLongRangeSim);
            _amtLimitLongRangeCycle = amt_cycle;
            assert(amt_cycle > 0);
        }

        bool is_long_rage_sim() const{return _isLongRangeSim;}

    };



}

#endif //KATHRYN_EVENTBASE_H
