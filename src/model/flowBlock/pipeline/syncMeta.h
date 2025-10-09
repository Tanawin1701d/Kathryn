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

        expression* _syncMasterReady = nullptr;
        expression* _syncSlaveReady  = nullptr;
        ///// typically the master should be zync block
        ///// typically the slave should be pipeline block
        expression* _syncMatched     = nullptr;

        ///// hold system signal representator
        std::vector<Operable*> masterHoldSignals;
        std::vector<Operable*> slaveHoldSignals;
        ///// reset system signal representator
        std::vector<Operable*> masterKillSignals;
        std::vector<Operable*> slaveKillSignals;
        ///// auto start
        std::vector<Operable*> slaveStartSignals;


        explicit SyncMeta(const std::string& name = "unnamedSyncMeta"):
        _name(name),
        _syncMasterReady(new expression(1)),
        _syncSlaveReady (new expression(1)),
        _syncMatched    (&((*_syncMasterReady) & (*_syncSlaveReady))){}

        ~SyncMeta()= default;

        std::string getName() const{
            return _name;
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
        }

        //////// restart is binded with basekiller typically it use the same signal with the base killer
        static Operable& baseStart(std::vector<Operable*>& desVector,
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