//
// Created by tanawin on 24/3/2567.
//

#include "pipeMng.h"
#include "model/controller/controller.h"

namespace kathryn{
    /**
     *
     * pipe meta
     *
     * **/

    Pipe::Pipe(PIPID pipeId):
    _pipeId(pipeId)
    {
        _masterReadyToSend = &makeOprProxyExpr("_masterReadyToSend" + std::to_string(_pipeId),1);
        _slaveReadyToRecv  = &makeOprProxyExpr("_slaveReadyToRecv"  + std::to_string(_pipeId),false,1);
    }

    Pipe::Pipe(const Pipe& rhs){
        if(this == (&rhs)){
            return;
        }
        operator=(rhs);
    }

    Pipe &Pipe::operator=(const Pipe &rhs) {
        if (this == (&rhs)){
            return *this;
        }

        _pipeId             = rhs._pipeId;
        _dummyStart         = rhs._dummyStart;
        _dummyStop          = rhs._dummyStop;
        _masterReadyToSend  = rhs._masterReadyToSend;
        _slaveReadyToRecv   = rhs._slaveReadyToRecv;

        return *this;
    }

    void Pipe::setDummyStartPipe(){
        _dummyStart = &makeOprVal("dummyStartPipe", 1,1);
        *_masterReadyToSend = *_dummyStart;
    }

    void Pipe::setDummyStopPipe(){
        _dummyStop = &makeOprVal("dummyStopPipe",1,1);
        *_slaveReadyToRecv = *_dummyStop;
    }

    /**
     *
     *
     * pipe controller
     *
     * */


    void PipeController::reset(){clean();}

    void PipeController::clean(){
        for(auto pipeMeta: _pipeMeta){
            delete pipeMeta;
        }
        _pipeMeta.clear();
    }

    Pipe& PipeController::createPipe() {
        Pipe* newPipe = new Pipe(getNextPipeId());
        _pipeMeta.push_back(newPipe);
        return *newPipe;
    }

    /**
     *
     * make pipe
     *
     * */

    Pipe& makePipe(){
        return getControllerPtr()->getPipeCtrl()->createPipe();
    }


    std::vector<Pipe*> makePipes(int amt){
        mfAssert(amt > 0,"build pipe must more than zero");
        std::vector<Pipe*> result;
        for (int i = 0; i < amt; i++){
            result.push_back(&makePipe());
        }
        return result;
    }


}