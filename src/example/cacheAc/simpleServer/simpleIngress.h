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

        explicit SimpleIngress(
            SERVER_PARAM                svParam,
            std::vector<BankInputInterface*> bankInterfaces):
        IngressBase(svParam, std::move(bankInterfaces)){}

        void flow() override{

            //////// initializing value and indexing
            var queueAvail = ~inputQueue.isEmpty();

            int startValueIdx = 0;
            int endValueIdx   = _svParam.kvParam.VALUE_SIZE;
            int startKeyIdx   = endValueIdx;
            int endKeyIdx     = startKeyIdx + _svParam.kvParam.KEY_SIZE;
            int startModeIdx  = endKeyIdx;
            /////////////////////////////////////////////////
            int stopIdent  = startKeyIdx + _svParam.kvParam.KEY_SIZE;
            int startIdent = startKeyIdx + _svParam.kvParam.KEY_SIZE - _svParam.prefixBit;


            //////// deal with bank interface
            for (int i = 0; i < _bankInterfaces.size(); i++){
                BankInputInterface& bankInItf = *_bankInterfaces[i];
                ////// assign reqToDequeue signal that it is ready to receive
                reqToDequeue(i) = bankInItf.isReqSuccess();
                ////// assign ready to send signal
                bankInItf.requestToSendOn(
                    queueAvail &
                    (inputQueue.getFront()(startIdent, stopIdent) == bankInItf.bankId)
                );
                bankInItf.setInputParam({
                       inputQueue.getFront()(startKeyIdx, endKeyIdx),
                       inputQueue.getFront()(startValueIdx, endValueIdx),
                       inputQueue.getFront()(startModeIdx)
                    });
            }

            /////// deal with queue

            cwhile(true){
                zif (reqToDequeue){
                    inputQueue.deQueue();
                }
            }
        }
    };

}

#endif //SIMPLEINGRESS_H
