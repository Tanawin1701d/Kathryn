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

        Operable* _syncMasterReady = nullptr;
        Operable* _syncSlaveReady  = nullptr;
        ///// typically the master should be zync block
        ///// typically the slave should be pipeline block
        Operable* _syncMatched     = nullptr;

        ///// hold system signal representator
        std::vector<Operable*> masterHoldSignals;
        std::vector<Operable*> slaveHoldSignals;

        expression& sendSuccess; //// TODO add this signal


        explicit SyncMeta(const std::string& name):
        _name(name),
        sendSuccess(*(new expression(1))){}

        ~SyncMeta()= default;

        std::string getName() const{
            return _name;
        }

        void buildMatchSignal(){
            assert(_syncMasterReady != nullptr);
            assert(_syncSlaveReady  != nullptr);
            assert(_syncMatched     == nullptr);
            assert(_syncMasterReady->getOperableSlice().getSize() == 1);
            assert(_syncSlaveReady->getOperableSlice().getSize()  == 1);

            _syncMatched = &((*_syncMasterReady) & (*_syncSlaveReady));
        }

        void tryBuildmatchSignal(){
            if (
            (_syncMasterReady != nullptr )&&
            (_syncSlaveReady  != nullptr )&&
            (_syncMatched     == nullptr )){
                buildMatchSignal();
            }
        }

        static void baseHolder(std::vector<Operable*>& desVector){
            mWire(slaveHolder, 1);
            slaveHolder = 1;
            desVector.push_back(&slaveHolder);
        }

        void holdSlave(){ baseHolder(slaveHoldSignals); }

        void holdMaster(){  baseHolder(masterHoldSignals); }

        void killSlave(){/** TODO*/}
        void killMaster(){/** TODO*/}

        Operable& getMatchStatus(){
            return sendSuccess;
        }





    };

}

#endif //KATHRYN_MODEL_FLOWBLOCK_PIPELINE_PIPEMETA_H