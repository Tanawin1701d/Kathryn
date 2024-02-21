//
// Created by tanawin on 21/2/2567.
//

#ifndef KATHRYN_LOGICSIMINTERFACE_H
#define KATHRYN_LOGICSIMINTERFACE_H

#include "modelSimInterface.h"

namespace kathryn{

    class LogicSimulatable : public Simulatable,
                           public RtlValItf{
    private:
        struct RTL_Meta_afterMf{
            //// data that require to init after model formation
            /// but before sim formation
            bool         _recCmd  = false;
            std::string  _recName = "UNDEFINED_SIGNAL_RECORD_NAME";
            VcdWriter*   _writer  = nullptr;
        };
    protected:
        bool               _simForNext   = false;
        bool               _isSimMetaSet = false;
        VCD_SIG_TYPE       _sigType = VST_DUMMY;
        RTL_Meta_afterMf   _simMeta;
        /** sz*/
        int                _sz;
        /** check that current and next cycle is simulate*/
        bool               _isCurValSim;
        bool               _isNextValSim;
        /** value that system is simulated */
        ValRep             _curVal;
        ValRep             _nextVal;
        /** idea we will use time array to store history of cycle
         *  but for now we use cur cycle and next cycle
         * */
    public:

        explicit LogicSimulatable(int sz, VCD_SIG_TYPE sigType, bool simForNext):
                _simForNext(simForNext),
                _isSimMetaSet(false),
                _sigType(sigType),
                _simMeta(RTL_Meta_afterMf()),
                _sz(sz),
                _isCurValSim(simForNext),
                _isNextValSim(false),
                _curVal(sz),
                _nextVal(sz),
                Simulatable(){
            assert(sz > 0);
        }

        ~LogicSimulatable() override = default;

        /** exit sim can be only invoked single time per cycle*/
        /***
         *
         * simulation action
         *
         * */

        /**specific set element before prepare sim*/
        virtual void beforePrepareSim(RTL_Meta_afterMf simMeta){
            _simMeta = std::move(simMeta);
        }

        /** before sim controller start prepare the system*/
        void prepareSim() override{
            assert(_isSimMetaSet);
            if (_simMeta._recCmd){
                _simMeta._writer->addNewVar(_sigType,
                                            _simMeta._recName,
                                            {0,_curVal.getLen()}
                );
            }
        }

        void simStartCurCycle() override{
            assert(false);
        }

        /**simulate next cycle value for current cycle*/
        void simStartNextCycle() override{
            assert(false);
        }

        /****/
        void curCycleCollectData() override{
            assert(_isSimMetaSet);
            if (_simMeta._recCmd){
                /** request for record*/
                assert(_simMeta._writer != nullptr);
                _simMeta._writer->addNewValue(_simMeta._recName, _curVal);
            }
        }

        /**we sure that it can be invoked only one*/
        void simExitCurCycle() override{
            assert(isCurValSim());
            _isCurValSim  = _isNextValSim;
            _isNextValSim = false;
            _curVal       = _nextVal;
            _nextVal      = ValRep(_sz);
        }

        /**
         *
         * simValue interface
         *
         * */

        void setCurValSimStatus () override{_isCurValSim  = true;}
        void setNextValSimStatus() override{_isNextValSim = true;}

        bool     isCurValSim () const override{ return _isCurValSim; };
        bool     isNextValSim() const override{ return _isNextValSim;};

        ValRep&  getCurVal () override {return _curVal; };
        ValRep&  getNextVal() override {return _nextVal;};

    };



}

#endif //KATHRYN_LOGICSIMINTERFACE_H
