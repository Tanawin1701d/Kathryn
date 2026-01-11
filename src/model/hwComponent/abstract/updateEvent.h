//
// Created by tanawin on 26/11/25.
//

#ifndef MODEL_HWCOMPONENT_ABSTRACT_ASSMETA_H
#define MODEL_HWCOMPONENT_ABSTRACT_ASSMETA_H

#include <vector>
#include <memory>

#include "operable.h"
#include "model/hwComponent/abstract/Slice.h"

#include "model/controller/clockMode.h"

#include "gen/proxyHwComp/abstract/updateEvent.h"
#include "sim/modelSimEngine/hwComponent/abstract/updateEvent.h"

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
    UpdateEventBasic* createUEHelper(Operable*  value,
                                     Slice      sl,
                                     int        priority,
                                     CLOCK_MODE cm,
                                     bool       autoPriority);

    UpdateEventCond* createUEHelper(Operable*        cond,
                                    Operable*        state,
                                    UpdateEventBase* ueb);

    // with condition
    UpdateEventBase* createUEHelper(Operable* cond,
                                  Operable*   state,
                                  Operable*   value,
                                  Slice       sl,
                                  int         priority,
                                  CLOCK_MODE  cm,
                                  bool        autoPriority);

    // create mux
    UpdateEventCond* createMuxUEHelper(UpdateEventBase* left,
                                       UpdateEventBase* right,
                                       Operable*        selectLeft);

    bool compareUE(const UpdateEventBase* lhs, const UpdateEventBase* rhs);

    struct UpdatePool{

        //////// event pool for each hardware component
        std::vector<UpdateEventBase*> events;

        void sortEvents();

        void addUpdateEvent(UpdateEventBase* event){
            //assert(event != nullptr);
            events.push_back(event);
        }

        std::vector<UpdateEventBase*>& getUpdateEventRef() {return events;}


        UpdatePool clone  ();
        void       clean  ();
        ///// check function
        Operable*  checkShortCircuitProxy() const;
        bool       isClockModeConsistent() const;
        ////// incase there is no updateEvent this item will return nullptr
        CLOCK_MODE getClockMode() const;
        size_t     size   () const{return events.size();}
        bool       isEmpty() const{return events.empty();}


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


        explicit UpdateEventBase(UE_TYPE type, bool isLeaf):
        _type(type),
        _isLeaf(isLeaf){}

        void setPriority   (int priority)      {_priority    = priority;}
        void setSubPriority(ull subPriority)   {_subPriority = subPriority;}
        void setClkMode    (CLOCK_MODE clkMode){_clkMode     = clkMode;}

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
        bool isJoinable(UpdateEventBase& rhs) const{return (_priority == rhs._priority) && (_clkMode == rhs._clkMode);}

        UE_TYPE    getType       () const {return _type;}
        int        getPriority   () const {return _priority;}
        ull        getSubPriority() const {return _subPriority;}
        CLOCK_MODE getClkMode    () const {return _clkMode;}
        //////// hardware checking
        virtual Operable* checkShortCircuitProxy() = 0;

        /////// mainly for simulation
        virtual void getDep(std::vector<Operable*>& resultDep) = 0;
        virtual UpdateEventBaseSimEngine* createSimEvent()     = 0;
        ////// mainly for generation
        virtual UpdateEventBase* clone()           = 0;
        virtual UEBaseGenEngine* createGenEngine() = 0;

    };

    struct UpdateEventBasic: UpdateEventBase{
        //// the instruction that push into one node
        Operable*  _value;
        Slice      _desSlice;


        UpdateEventBasic(Operable* val, const Slice& slice, int priority, CLOCK_MODE clkMode) :
        UpdateEventBase(UET_BASIC, true),
        _value   (val),
        _desSlice(slice){
            assert(val != nullptr);
            setPriority(priority);
            setClkMode(clkMode);
        }

        Operable* checkShortCircuitProxy() override{
            return _value->checkShortCircuit();
        }

        ///////// for simulation generation
        void getDep(std::vector<Operable*>& resultDep) override{
            resultDep.push_back(_value);
        }

        UpdateEventBaseSimEngine*  createSimEvent() override{
            return new UpdateEventBasicSimEngine(this);
        }
        ///////// for verilog generation
        UpdateEventBase* clone() override{
            auto* ueb = new UpdateEventBasic(*this);
            return ueb;
        }
        UEBaseGenEngine* createGenEngine() override {
            return new UEBasicGenEngine(this);
        }

    };

    struct UpdateEventGrp: UpdateEventBase{
        std::vector<UpdateEventBase*> subStmts;

        UpdateEventGrp():
        UpdateEventBase(UET_GRP, false){}

        void addSubStmt(UpdateEventBase* stmt){
            assert(stmt != nullptr);
            if (subStmts.empty()){
                setPriority(stmt->_priority);
                setClkMode (stmt->_clkMode);
            }
            subStmts.push_back(stmt);
        }
        Operable* checkShortCircuitProxy() override{
            Operable* result = nullptr;
            for (auto* stmt: subStmts){
                result =  stmt->checkShortCircuitProxy();
                if (result != nullptr){
                    return result;
                }
            }
            return result;
        }
        ///////// for simulation generation
        void getDep(std::vector<Operable*>& resultDep) override{
            for (auto* stmt: subStmts){
                stmt->getDep(resultDep);
            }
        }
        UpdateEventBaseSimEngine*  createSimEvent() override{
            return new UpdateEventGrpSimEngine(this);
        }
        ///////// for verilog generation
        UpdateEventBase* clone() override{
            auto* ueb = new UpdateEventGrp(*this);
            for (int idx = 0; idx < subStmts.size(); idx++){
                //// it is ok about the vector size because it is cloned already
                ueb->subStmts[idx] = subStmts[idx]->clone();
            }
            return ueb;
        }
        UEBaseGenEngine* createGenEngine() override {
            return new UEGrpGenEngine(this);
        }

    };

    struct UpdateEventCond: UpdateEventBase{
        bool _isLastOccure = false;
        //// the index of condition and subStmt is refer to the same block
        std::vector<Operable*>        conditions;
        std::vector<UpdateEventBase*> subStmts;

        UpdateEventCond():
        UpdateEventBase(UET_COND, false){}

        void addSubStmt(Operable* cond, UpdateEventBase* stmt){
            assert(stmt != nullptr);
            assert(!_isLastOccure);
            if (cond == nullptr){
                _isLastOccure = true;
            }
            if (subStmts.empty()){
                setPriority(stmt->_priority);
                setClkMode(stmt->_clkMode);
            }
            conditions.push_back(cond);
            subStmts  .push_back(stmt);

        }

        Operable* checkShortCircuitProxy() override{
            Operable* result = nullptr;
            for (auto* cond: conditions){
                if (cond != nullptr){
                    result = cond->checkShortCircuit();
                    if (result != nullptr){
                        return result;
                    }
                }
            }
            for (auto* stmt: subStmts){
                result = stmt->checkShortCircuitProxy();
                if (result != nullptr){
                    return result;
                }
            }
            return result;
        }
        ///////// for simulation generation

        void getDep(std::vector<Operable*>& resultDep) override{
            for (auto* cond: conditions){
                if (cond != nullptr){
                    resultDep.push_back(cond);
                }
            }
            for (auto* stmt: subStmts){
                stmt->getDep(resultDep);
            }
        }

        UpdateEventBaseSimEngine*  createSimEvent() override{
            return new UpdateEventCondSimEngine(this);
        }

        ///////// for verilog generation
        UpdateEventBase* clone() override{
            auto* ueb = new UpdateEventCond(*this);
            for (int idx = 0; idx < subStmts.size(); idx++){
                //// it is ok about the vector size because it is cloned already
                ueb->subStmts[idx] = subStmts[idx]->clone();
            }

            return ueb;
        }
        UEBaseGenEngine* createGenEngine() override {
            return new UECondGenEngine(this);
        }

    };


    struct UpdateEventSwitch: UpdateEventBase{
        Operable& stateIden;
        /////// incase
        std::vector<int>              subStmtIdxs;
        std::vector<UpdateEventBase*> subStmts;

        explicit UpdateEventSwitch(Operable& stateIden):
        UpdateEventBase(UET_SWITCH, false),
        stateIden(stateIden){
        }

        int getMaxIdx() const{
            return 1 << stateIden.getOperableSlice().getSize();
        }

        int getMatchNum()const{
            assert(subStmtIdxs.size() == subStmts.size());
            return subStmtIdxs.size();
        }

        Operable& getStateIdent(){
            return stateIden;
        }

        int getSubStmtMatchIdxs(int idx) const{
            assert(idx < subStmtIdxs.size());
            return subStmtIdxs[idx];
        }

        UpdateEventBase* getSubStmts(int idx) const{
            assert(idx < subStmts.size());
            return subStmts[idx];
        }

        void addSubStmt(int matchVal, UpdateEventBase* stmt){

            assert(stmt      != nullptr);
            assert( (matchVal >= -1) && (matchVal < getMaxIdx()));
            if (subStmtIdxs.empty()){
                setPriority(stmt->_priority);
                setClkMode (stmt->_clkMode);
            }
            subStmtIdxs.push_back(matchVal);
            subStmts   .push_back(stmt);

        }

        Operable* checkShortCircuitProxy() override{
            Operable* result = nullptr;
            result = stateIden.checkShortCircuit();
            if (result != nullptr){
                return result;
            }
            for (auto* stmt: subStmts){
                result = stmt->checkShortCircuitProxy();
                if (result != nullptr){
                    return result;
                }
            }
            return result;
        }
        ///////// for simulation generation

        void getDep(std::vector<Operable*>& resultDep) override{
            resultDep.push_back(&stateIden);
            for (auto* stmt: subStmts){
                stmt->getDep(resultDep);
            }
        }

        UpdateEventBaseSimEngine*  createSimEvent() override{
            return new UpdateEventSwitchSimEngine(this);
        }

        ///////// for verilog generation
        UpdateEventBase* clone() override{
            auto* ueb = new UpdateEventSwitch(*this);
            for (int idx = 0; idx < subStmts.size(); idx++){
                ueb->subStmts[idx] = subStmts[idx]->clone();
            }
            return ueb;
        }
        UEBaseGenEngine* createGenEngine() override {
            return new UESwitchGenEngine(this);
        }

    };

}

#endif //MODEL_HWCOMPONENT_ABSTRACT_ASSMETA_H