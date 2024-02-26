//
// Created by tanawin on 19/2/2567.
//

#ifndef KATHRYN_MEMSIMINTERFACE_H
#define KATHRYN_MEMSIMINTERFACE_H

#include "sim/logicRep/valRep.h"
#include "modelSimInterface.h"

namespace kathryn{


    class MemSimEngine : public SimEngine{
    protected:
        /** we will not use sim engine anymore*/
        ValRep* memBlk = nullptr;

        bool               _isCurValSim  = false;
        bool               _isNextValSim = false;

        const ull DEPTH_SIZE = 0;
        const int WIDTH_SIZE = 0;


    public:
        MemSimEngine(ull depthSize, int widthSize);
        ~MemSimEngine(){
            delete[] memBlk;
        }

        //////// simulatable override
        void prepareSim() override;

        void curCycleCollectData() override {assert(false);};

        ValRep& getThisCycleValRep(ull idx);

        //////// rtlValItf override
        void setCurValSimStatus () override{_isCurValSim  = true;}
        void setNextValSimStatus() override{_isNextValSim = true;}

        bool     isCurValSim () const override{ return _isCurValSim; };
        bool     isNextValSim() const override{ return _isNextValSim;};

        ValRep&  getCurVal () override {
            assert(false);
        }

        ValRep&  getNextVal() override {
            assert(false);
        }



    };


    class MemAgentSimEngine : public SimEngine{
    protected:
        bool               _isCurValSim  = false;
        bool               _isNextValSim = false;

        ValRep* _curAgentVal = nullptr;
        ValRep  _nextAgentVal;
        ////// prepare sim

    public:

        explicit MemAgentSimEngine(int bitWidth):
        _nextAgentVal(bitWidth)
        {}

        void prepareSim() override{
            assert(false);
        }

        void curCycleCollectData() override{
            assert(false);
        }

        void simExitCurCycle() override{
            assert(_curAgentVal != nullptr);
            /** transfer simStatus*/
            _isCurValSim  = false;
            _isNextValSim = false;
            if (!isReadMode()){ //// write mode
                *_curAgentVal = _nextAgentVal;
            }
            ////// may be there is no
        }

        virtual bool isReadMode() = 0;
        //// the read/write mode will be balid when prepare sim was invoked.

        void setCurValSimStatus () override{_isCurValSim  = true;}
        void setNextValSimStatus() override{_isNextValSim = true;}

        bool     isCurValSim () const override{ return _isCurValSim; };
        bool     isNextValSim() const override{ return _isNextValSim;};

        ValRep&  getCurVal () override {
            assert(_curAgentVal != nullptr);
            return *_curAgentVal;
        }

        ValRep&  getNextVal() override {
            return _nextAgentVal;
        }
    };




}

#endif //KATHRYN_MEMSIMINTERFACE_H
