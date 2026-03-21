//
// Created by tanawin on 26/11/25.
//

#ifndef MODEL_HWCOMPONENT_ABSTRACT_ASSMETA_H
#define MODEL_HWCOMPONENT_ABSTRACT_ASSMETA_H

#include "vector"
#include "memory"

#include "operable.h"
#include "model/hw_component/abstract/slice.h"

#include "model/controller/clock_mode.h"

#include "gen/proxy_hw_comp/abstract/update_event.h"
#include "sim/model_sim_engine/hw_component/abstract/update_event.h"

namespace kathryn{

    /** default -->update -->event -->priority */
    static int DEFAULT_UE_PRI_USER          = 10;
    static int DEFAULT_UE_PRI_INTERNAL_MAX  = 100;
    static int DEFAULT_UE_PRI_INTERNAL_MIN  = 50;
    static int DEFAULT_UE_PRI_RST           = INT32_MAX;
    static int DEFAULT_UE_PRI_MIN           = 0;
    static ull DEFAULT_UE_SUB_PRIORITY_USER = 0;

    struct UpdateEventBase;
    struct UpdateEventBasic;
    struct UpdateEventCond;



    // without condition
    UpdateEventBasic* create_ue_helper(Operable*  value,
                                     Slice      sl,
                                     int        priority,
                                     CLOCK_MODE cm,
                                     bool       auto_priority);

    UpdateEventCond* create_ue_helper(Operable*        cond,
                                    Operable*        state,
                                    UpdateEventBase* ueb);

    // with condition
    UpdateEventBase* create_ue_helper(Operable* cond,
                                  Operable*   state,
                                  Operable*   value,
                                  Slice       sl,
                                  int         priority,
                                  CLOCK_MODE  cm,
                                  bool        auto_priority);

    // create mux
    UpdateEventCond* create_mux_ue_helper(UpdateEventBase* left,
                                       UpdateEventBase* right,
                                       Operable*        select_left);

    bool compare_ue(const UpdateEventBase* lhs, const UpdateEventBase* rhs);

    struct UpdatePool{

        //////// event pool for each hardware component
        std::vector<UpdateEventBase*> events;

        void sort_events();

        void add_update_event(UpdateEventBase* event){
            //assert(event != nullptr);
            events.push_back(event);
        }

        std::vector<UpdateEventBase*>& get_update_event_ref() {return events;}


        UpdatePool clone  ();
        void       clean  ();
        ///// check function
        Operable*  check_short_circuit_proxy() const;
        bool       is_clock_mode_consistent() const;
        ////// incase there is no update_event this item will return nullptr
        CLOCK_MODE get_clock_mode() const;
        size_t     size   () const{return events.size();}
        bool       is_empty() const{return events.empty();}


    };

    enum UE_TYPE{
        UET_BASIC = 0,
        UET_GRP   = 1,
        UET_COND  = 2,
        UET_SWITCH= 3
    };

    struct UpdateEventBase{
        virtual ~UpdateEventBase() = default;
        ////// the base line of the assignment
        UE_TYPE    _type        = UET_BASIC;
        bool       _isLeaf      = false;
        int        _priority    = DEFAULT_UE_PRI_MIN;
        ull        _subPriority = DEFAULT_UE_SUB_PRIORITY_USER;
        CLOCK_MODE _clkMode     = CM_CLK_UNUSED;


        explicit UpdateEventBase(UE_TYPE type, bool is_leaf):
        _type(type),
        _isLeaf(is_leaf){}

        void set_priority   (int priority)      {_priority    = priority;}
        void set_sub_priority(ull sub_priority)   {_subPriority = sub_priority;}
        void set_clk_mode    (CLOCK_MODE clk_mode){_clkMode     = clk_mode;}

        bool operator < (const UpdateEventBase& rhs) const{
            if (_priority < rhs._priority){
                return true;
            }
            if (_priority == rhs._priority){
                return _subPriority < rhs._subPriority;
            }
            return false;
        }

        //// it used to tell update Event can be joined into single simulation or generation block
        bool is_joinable(UpdateEventBase& rhs) const{return (_priority == rhs._priority) && (_clkMode == rhs._clkMode);}

        UE_TYPE    get_type       () const {return _type;}
        int        get_priority   () const {return _priority;}
        ull        get_sub_priority() const {return _subPriority;}
        CLOCK_MODE get_clk_mode    () const {return _clkMode;}
        //////// hardware checking
        virtual Operable* check_short_circuit_proxy() = 0;

        /////// mainly for simulation
        virtual void get_dep(std::vector<Operable*>& result_dep) = 0;
        virtual UpdateEventBaseSimEngine* create_sim_event()     = 0;
        ////// mainly for generation
        virtual UpdateEventBase* clone()           = 0;
        virtual UEBaseGenEngine* create_gen_engine() = 0;

    };

    struct UpdateEventBasic: UpdateEventBase{
        //// the instruction that push into one node
        Operable*  _value;
        Slice      _desSlice;


        UpdateEventBasic(Operable* val, const Slice& slice, int priority, CLOCK_MODE clk_mode) :
        UpdateEventBase(UET_BASIC, true),
        _value   (val),
        _desSlice(slice){
            assert(val != nullptr);
            set_priority(priority);
            set_clk_mode(clk_mode);
        }

        Operable* check_short_circuit_proxy() override{
            return _value->check_short_circuit();
        }

        ///////// for simulation generation
        void get_dep(std::vector<Operable*>& result_dep) override{
            result_dep.push_back(_value);
        }

        UpdateEventBaseSimEngine*  create_sim_event() override{
            return new UpdateEventBasicSimEngine(this);
        }
        ///////// for verilog generation
        UpdateEventBase* clone() override{
            auto* ueb = new UpdateEventBasic(*this);
            return ueb;
        }
        UEBaseGenEngine* create_gen_engine() override {
            return new UEBasicGenEngine(this);
        }

    };

    struct UpdateEventGrp: UpdateEventBase{
        std::vector<UpdateEventBase*> sub_stmts;

        UpdateEventGrp():
        UpdateEventBase(UET_GRP, false){}

        void add_sub_stmt(UpdateEventBase* stmt){
            assert(stmt != nullptr);
            if (sub_stmts.empty()){
                set_priority(stmt->_priority);
                set_clk_mode (stmt->_clkMode);
            }
            sub_stmts.push_back(stmt);
        }
        Operable* check_short_circuit_proxy() override{
            Operable* result = nullptr;
            for (auto* stmt: sub_stmts){
                result =  stmt->check_short_circuit_proxy();
                if (result != nullptr){
                    return result;
                }
            }
            return result;
        }
        ///////// for simulation generation
        void get_dep(std::vector<Operable*>& result_dep) override{
            for (auto* stmt: sub_stmts){
                stmt->get_dep(result_dep);
            }
        }
        UpdateEventBaseSimEngine*  create_sim_event() override{
            return new UpdateEventGrpSimEngine(this);
        }
        ///////// for verilog generation
        UpdateEventBase* clone() override{
            auto* ueb = new UpdateEventGrp(*this);
            for (int idx = 0; idx < sub_stmts.size(); idx++){
                //// it is ok about the vector size because it is cloned already
                ueb->sub_stmts[idx] = sub_stmts[idx]->clone();
            }
            return ueb;
        }
        UEBaseGenEngine* create_gen_engine() override {
            return new UEGrpGenEngine(this);
        }

    };

    struct UpdateEventCond: UpdateEventBase{
        bool _isLastOccure = false;
        //// the index of condition and sub_stmt is refer to the same block
        std::vector<Operable*>        conditions;
        std::vector<UpdateEventBase*> sub_stmts;

        UpdateEventCond():
        UpdateEventBase(UET_COND, false){}

        void add_sub_stmt(Operable* cond, UpdateEventBase* stmt){
            assert(stmt != nullptr);
            assert(!_isLastOccure);
            if (cond == nullptr){
                _isLastOccure = true;
            }
            if (sub_stmts.empty()){
                set_priority(stmt->_priority);
                set_clk_mode(stmt->_clkMode);
            }
            conditions.push_back(cond);
            sub_stmts  .push_back(stmt);

        }

        Operable* check_short_circuit_proxy() override{
            Operable* result = nullptr;
            for (auto* cond: conditions){
                if (cond != nullptr){
                    result = cond->check_short_circuit();
                    if (result != nullptr){
                        return result;
                    }
                }
            }
            for (auto* stmt: sub_stmts){
                result = stmt->check_short_circuit_proxy();
                if (result != nullptr){
                    return result;
                }
            }
            return result;
        }
        ///////// for simulation generation

        void get_dep(std::vector<Operable*>& result_dep) override{
            for (auto* cond: conditions){
                if (cond != nullptr){
                    result_dep.push_back(cond);
                }
            }
            for (auto* stmt: sub_stmts){
                stmt->get_dep(result_dep);
            }
        }

        UpdateEventBaseSimEngine*  create_sim_event() override{
            return new UpdateEventCondSimEngine(this);
        }

        ///////// for verilog generation
        UpdateEventBase* clone() override{
            auto* ueb = new UpdateEventCond(*this);
            for (int idx = 0; idx < sub_stmts.size(); idx++){
                //// it is ok about the vector size because it is cloned already
                ueb->sub_stmts[idx] = sub_stmts[idx]->clone();
            }

            return ueb;
        }
        UEBaseGenEngine* create_gen_engine() override {
            return new UECondGenEngine(this);
        }

    };


    struct UpdateEventSwitch: UpdateEventBase{
        bool      is_init_meta = false; ///// some time this switch event will be init with nullptr
                                     ////// substmts to maintain the free switch matched case
                                     ////// therefore the meta data may be not initialized at that time
        Operable* state_iden;
        /////// incase
        std::vector<int>              sub_stmt_idxs;
        std::vector<UpdateEventBase*> sub_stmts;

        explicit UpdateEventSwitch(Operable* state_iden):
        UpdateEventBase(UET_SWITCH, false),
        state_iden(state_iden){
        }

        int get_max_idx() const{
            return 1 << state_iden->get_operable_slice().get_size();
        }

        int get_match_num()const{
            assert(sub_stmt_idxs.size() == sub_stmts.size());
            return sub_stmt_idxs.size();
        }

        Operable*get_state_ident_ptr(){
            return state_iden;
        }

        Operable*& get_state_ident_ref(){
            return state_iden;
        }

        int get_sub_stmt_match_idxs(int idx) const{
            assert(idx < sub_stmt_idxs.size());
            return sub_stmt_idxs[idx];
        }

        UpdateEventBase*get_sub_stmts_ptr(int idx) const{
            assert(idx < sub_stmts.size());
            return sub_stmts[idx];
        }

        void add_sub_stmt(int match_val, UpdateEventBase* stmt){

            //assert(stmt      != nullptr);  nullptr mean dummy operation in this case
            assert( (match_val >= -1) && (match_val < get_max_idx()));
            if (!is_init_meta && (stmt != nullptr)){
                set_priority(stmt->_priority);
                set_clk_mode (stmt->_clkMode);
                is_init_meta = true;
            }
            sub_stmt_idxs.push_back(match_val);
            sub_stmts   .push_back(stmt);

        }

        Operable* check_short_circuit_proxy() override{
            Operable* result = nullptr;
            result = state_iden->check_short_circuit();
            if (result != nullptr){
                return result;
            }
            for (auto* stmt: sub_stmts){
                if (stmt == nullptr){continue;}
                result = stmt->check_short_circuit_proxy();
                if (result != nullptr){
                    return result;
                }
            }
            return result;
        }
        ///////// for simulation generation

        void get_dep(std::vector<Operable*>& result_dep) override{
            result_dep.push_back(state_iden);
            for (auto* stmt: sub_stmts){
                if (stmt == nullptr){continue;}
                stmt->get_dep(result_dep);
            }
        }

        UpdateEventBaseSimEngine*  create_sim_event() override{
            return new UpdateEventSwitchSimEngine(this);
        }

        ///////// for verilog generation
        UpdateEventBase* clone() override{
            auto* ueb = new UpdateEventSwitch(*this);
            for (int idx = 0; idx < sub_stmts.size(); idx++){
                if (ueb->sub_stmts[idx] == nullptr){continue;}
                ueb->sub_stmts[idx] = sub_stmts[idx]->clone();
            }
            return ueb;
        }
        UEBaseGenEngine* create_gen_engine() override {
            return new UESwitchGenEngine(this);
        }

    };

}

#endif //MODEL_HWCOMPONENT_ABSTRACT_ASSMETA_H