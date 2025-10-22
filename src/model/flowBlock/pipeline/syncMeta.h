//
// Created by tanawin on 13/9/25.
//

#ifndef KATHRYN_MODEL_FLOWBLOCK_PIPELINE_PIPEMETA_H
#define KATHRYN_MODEL_FLOWBLOCK_PIPELINE_PIPEMETA_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

#define mZync(name) SyncMeta name{#name}


namespace kathryn{

    ///// sync is used to sync transfer data between
    ///   master (zync block) and slave (pip block)
    struct SyncMeta{

        const std::string _name;

        ///// main synchronizer
        mWire(_syncMasterReady, 1);
        mWire(_syncSlaveReady , 1) ;
        ///// slave ready after execution
        mWire(_syncSlaveFin, 1);

        ///// typically the master should be zync block
        ///// typically the slave should be pipeline block
        mWire(_syncMatched, 1);

        mWire(killSlaveSignal  , 1);
        mWire(killMasterSignal , 1);
        mWire(holdSlaveSignal   , 1);
        mWire(holdMasterSignal , 1);
        mWire(startSlaveSignal , 1);


        explicit SyncMeta(const std::string& name = "unnamedSyncMeta"):
        _name(name){
            setSyncWireBase(_syncMatched, _syncMasterReady & _syncSlaveReady);
        }

        ~SyncMeta()= default;

        std::string getName() const{ return _name;}

        ////// to ask that is slave fin in this cycle
        Operable& isSlaveFin(){return _syncSlaveFin;}

        void setSyncWireBase(Wire& desWire, Operable& opr1){
            UpdateEvent* newEvent = new UpdateEvent({
                nullptr,
                nullptr,
                &opr1,
                Slice({0, 1}),
                DEFAULT_UE_PRI_INTERNAL_MIN,
                DEFAULT_UE_SUB_PRIORITY_USER,
                CM_CLK_FREE
            });
            desWire.getUpdateMeta().push_back(newEvent);
        }

        /*
         * INTERNAL SIGNAL USED BY internal node belonging PIPE ZYNC node
         */

        ////// set start signal
        void setMasterReady(Operable& opr1){
            mfAssert(opr1.getOperableSlice().getSize() == 1, "setMasterReady size must be 1");
            setSyncWireBase(_syncMasterReady, opr1);
        }

        void setSlaveReady(Operable& opr1){
            mfAssert(opr1.getOperableSlice().getSize() == 1, "setSlaveReady size must be 1");
            setSyncWireBase(_syncSlaveReady, opr1);
        }

        void setSlaveFinish(Operable& opr1){
            mfAssert(opr1.getOperableSlice().getSize() == 1, "setSlaveReady size must be 1");
            setSyncWireBase(_syncSlaveFin, opr1);
        }

        /////// for use use signal

        void holdSlave(){ holdSlaveSignal = 1; }

        void holdMaster(){ holdMasterSignal = 1; }

        void restartSlave(Operable* cond = nullptr){
            if (cond == nullptr){
                startSlaveSignal = 1;
            }else{
                assert(cond->getOperableSlice().getSize() == 1);
                startSlaveSignal = *cond;
            }
        }

        void killSlave(bool autoRestart = false, Operable* cond = nullptr){

            if(cond == nullptr){
                killSlaveSignal = 1;
            }else{
                assert(cond->getOperableSlice().getSize() == 1);
                killSlaveSignal = *cond;
            }
            if (autoRestart){
                restartSlave(cond);
            }

        }

        void killMaster(){
            killMasterSignal = 1;
        }

    };

}

#endif //KATHRYN_MODEL_FLOWBLOCK_PIPELINE_PIPEMETA_H