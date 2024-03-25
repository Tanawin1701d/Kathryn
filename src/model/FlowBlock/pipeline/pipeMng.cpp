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

    Pipe::Pipe(bool allocRequire){

        if (allocRequire){
            alloc();
        }


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
        mfAssert(rhs.isAlloc(), "copy pipe meta data while it is not allocated can lead to unexpect connection");

        _isAlloc            = true;
        _pipeId             = rhs._pipeId;
        _dummyVal           = rhs._dummyVal;
        _availSendSignal    = rhs._availSendSignal;
        _notifyToSendSignal = rhs._notifyToSendSignal;

        return *this;
    }

    void Pipe::alloc(){
        PipeController* pipCtrl = getControllerPtr()->getPipeCtrl();
        pipCtrl->allocPipe(this);
        ///////// pipId and alloc meta controller wil handdle it
        _availSendSignal    = &_make<expression>("availSendSignal_" + std::to_string(_pipeId),1);
        _notifyToSendSignal = &_make<expression>("notifyToSendSignal_" + std::to_string(_pipeId),1);
    }

    bool Pipe::isAlloc() const{
        return _isAlloc;
    }

    void Pipe::reverse(){
        std::swap(_availSendSignal, _notifyToSendSignal);
    }

    void Pipe::setAsDummyPipe(){
        mfAssert(isAlloc(), "you must allocate before set dummy pipe");
        _dummyVal = &_make<Val>("dummyStartPipe", 1,0);
        *_availSendSignal = *_dummyVal;
    }

    /**
     *
     *
     * pipe controller
     *
     * */


    void PipeController::reset(){clean();}

    void PipeController::clean(){
        _pipeMeta.clear();
    }

    pipId PipeController::allocPipe(Pipe* newPipeCom) {
        assert(newPipeCom != nullptr);
        newPipeCom->_pipeId  = getNextPipeId();
        newPipeCom->_isAlloc = true;
        _pipeMeta.push_back(*newPipeCom);
        return newPipeCom->_pipeId;
    }


}