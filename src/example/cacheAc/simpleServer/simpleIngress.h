//
// Created by tanawin on 15/9/2024.
//

#ifndef SIMPLEINGRESS_H
#define SIMPLEINGRESS_H

#include <utility>

#include "example/cacheAc/ingress.h"


namespace kathryn::cacheServer{

    class SimpleIngress: public IngressBase{
    public:



        int startValueIdx = 0;
        int endValueIdx   = _svParam.kvParam.VALUE_SIZE;
        int startKeyIdx   = endValueIdx;
        int endKeyIdx     = startKeyIdx + _svParam.kvParam.KEY_SIZE;
        int startModeIdx  = endKeyIdx;
        /////////////////////////////////////////////////
        int stopIdent  = startKeyIdx + _svParam.kvParam.KEY_SIZE;
        int startIdent = startKeyIdx + _svParam.kvParam.KEY_SIZE - _svParam.prefixBit;

        Wire& frontSig; ////// |isLoad|key|value|
        Operable& frontKey;
        Operable& frontValue;
        Operable& frontIsload;
        Operable& frontBankIdent;

        explicit SimpleIngress(
            SERVER_PARAM                svParam,
            std::vector<BankInputInterface*> bankInterfaces):
        IngressBase(svParam, std::move(bankInterfaces)),
        frontSig(inputQueue.getFront()),
        frontKey      (frontSig(startKeyIdx  , endKeyIdx   )),
        frontValue    (frontSig(startValueIdx, endValueIdx )),
        frontIsload   (frontSig(startModeIdx                     )),
        frontBankIdent(frontSig(startIdent   , stopIdent   )){

            for (auto & _bankInterface : _bankInterfaces)
                _bankInterface->setInputParam({frontKey,frontValue,frontIsload});

        }

        void flow() override{

            //////// initializing value and indexing
            var queueAvail = ~inputQueue.isEmpty();
            //////// deal with bank interface
            for (int i = 0; i < _bankInterfaces.size(); i++){
                BankInputInterface* bankInItf = _bankInterfaces[i];
                ////// assign reqToDequeue signal that it is ready to receive
                reqToDequeue(i) = bankInItf->isReqSuccess();
                ////// assign ready to send signal
                bankInItf->requestToSendOn(queueAvail & (frontBankIdent == bankInItf->bankId));
            }
            /////// deal with queue
            inputQueue.initLogic();
            cwhile(true){
                zif (reqToDequeue){
                    inputQueue.deQueue();
                }
            }
        }
    };

}

#endif //SIMPLEINGRESS_H
