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
    ///   master and slave
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

        ///// hold system signal representator
        std::vector<Operable*> masterHoldSignals;
        std::vector<Operable*> slaveHoldSignals;
        ///// reset system signal representator
        std::vector<Operable*> masterKillSignals;
        std::vector<Operable*> slaveKillSignals;
        ///// auto start
        std::vector<Operable*> slaveStartSignals;


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

        static void baseHolder(std::vector<Operable*>& desVector){
            mWire(holder, 1);
            holder = 1;
            desVector.push_back(&holder);
        }

        void holdSlave(){ baseHolder(slaveHoldSignals); }

        void holdMaster(){  baseHolder(masterHoldSignals); }

        static Operable& baseKiller(std::vector<Operable*>& desVector,
                               Operable* cond = nullptr){
            mWire(killer, 1);
            if (cond != nullptr){
                killer = *cond;
            }else{
                killer = 1;
            }
            desVector.push_back(&killer);
            return killer;
        }

        //////// restart is binded with basekiller typically it use the same signal with the base killer
        static void baseStart(std::vector<Operable*>& desVector,
                                   Operable& signal){
            desVector.push_back(&signal);
        }

        void killSlave(bool autoRestart = false, Operable* cond = nullptr){
            Operable& killSignal = baseKiller(slaveKillSignals, cond);
            if (autoRestart){
                baseStart(slaveStartSignals, killSignal);
            }

        }
        void killMaster(){ baseKiller(masterKillSignals, nullptr);}

    };

}



// void buildMatchSignal(){
//     assert(_syncMasterReady != nullptr);
//     assert(_syncSlaveReady  != nullptr);
//     assert(_syncMatched     == nullptr);
//     assert(_syncMasterReady->getOperableSlice().getSize() == 1);
//     assert(_syncSlaveReady->getOperableSlice().getSize()  == 1);
//
//     _syncMatched = &((*_syncMasterReady) & (*_syncSlaveReady));
// }

// void tryBuildmatchSignal(){
//     if (
//     (_syncMasterReady != nullptr )&&
//     (_syncSlaveReady  != nullptr )&&
//     (_syncMatched     == nullptr )){
//         buildMatchSignal();
//     }
// }

#endif //KATHRYN_MODEL_FLOWBLOCK_PIPELINE_PIPEMETA_H