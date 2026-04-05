//
// Created by tanawin on 3/10/2024.
//

#include "model/flowBlock/cond/zif.h"
#include "model/flowBlock/cond/zelif.h"
#include "model/flowBlock/loop/doWhileBase.h"
#include "shs.h"

namespace kathryn{

    void SingleHandShakeBase::buildLogicBase(){
        if (_autoAccept){
            reqResult = (~isBusy | readyToGetNew) & reqToSend;
        }else{
            reqResult = readyToGetNew & reqToSend;
        }
        zif(reqResult){
            isBusy <<= 1;
            transferPayLoad();
        }zelif(readyToGetNew){
            isBusy <<= 0;
        }
        buildUserLogic();
    }

    void SingleHandShakeBase::sendAndWaitUntillSuccess(){

        cdowhile(~isReqSuccess()){
            send();
        }

    }

}