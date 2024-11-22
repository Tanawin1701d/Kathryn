//
// Created by tanawin on 22/11/2024.
//

#include "mtAgent.h"

namespace kathryn{



void MtCtrlAgent::masterCtrlReqSuspend(){
    susReq <<= 1;
}

void MtCtrlAgent::masterCtrlReqStart(Operable& tid){
    par {
        nextTid <<= tid;
        startReq <<= 1;
    }
}

void MtCtrlAgent::modelReqSuspend(){susReq = 1;}

Operable& MtCtrlAgent::getRunningStatus(){return modelIsRunning;}

Operable& MtCtrlAgent::isReqMcToInScp(){return reqMcToInScp;}
Operable& MtCtrlAgent::isReqInScpToMd(){return reqInScpToMd;}
Operable& MtCtrlAgent::isReqMdToOutScp(){return reqMdToOutScp;}
Operable& MtCtrlAgent::isReqOutScpToMc(){return reqOutScpToMc;}

void      MtCtrlAgent::ackMcToInScp()  {finMcToInScp  = 1;}
void      MtCtrlAgent::ackInScpToMd()  {finInScpToMd  = 1;}
void      MtCtrlAgent::ackMdToOutScp() {finMdToOutScp  = 1;}
void      MtCtrlAgent::ackOutScpToMc() {finOutScpToMc = 1;}

void MtCtrlAgent::flow(){
    buildModelCtrl();
    buildInDataCtrl();
    buildOutDataCtrl();
}

void MtCtrlAgent::buildModelCtrl(){
////////// control the model ///////////////////////
    seq cwhile(true){
            cif(startReq) {
                /////// req scratch pad to model //////////
                scWait(nextTidInScpReady);
                par{
                    startReq          <<= 0; /// trip req off
                    curTid            <<= nextTid;
                    nextTid           <<= 0;
                    nextTidInScpReady <<= 0;
                    reqInScpToMd      = 1;
                }
                scWait(finInScpToMd);
                //////////////////////////////////////////
                cdowhile(~susReq){
                    modelIsRunning = 1; //// running
                }
                /////// req model back to sc pad ///////////
                scWait(~outScpHaveData)
                par{
                    susReq         <<= 0;
                    curTid         <<= 0;
                    outScpHaveData <<= 1;
                    reqMdToOutScp  =   1;
                }
                scWait(finMdToOutScp);

                //////////////////////////////////////////
            }celse{
                syWait(1);
            };
        }
    //////////////////////////////////////////////////
}

void MtCtrlAgent::buildInDataCtrl() {

    ////// this function manage the data to get in to the
    ////// from memory controller to input scratch pad

    seq cwhile(true){
        cif((nextTid != 0) && (~nextTidInScpReady)){
            cdowhile(~finMcToInScp)
                nextTidInScpReady <<= finMcToInScp;
        }celse{
            syWait(1)
        }
    }


}

void MtCtrlAgent::buildOutDataCtrl() {

    ////// this function manage the data to get out to the
    ////// scratch pad to memory controller

    cdowhile(~finOutScpToMc){
        reqOutScpToMc = outScpHaveData;
        zif (finOutScpToMc)
            outScpHaveData <<= 0;
    }

}

}