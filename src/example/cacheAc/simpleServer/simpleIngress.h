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
            SERVER_PARAM                     svParam,
            std::vector<BankInputInterface*> bankInterfaces):
        IngressBase(svParam, std::move(bankInterfaces),
        {"isLoad", "key", "value"},
        {1, svParam.kvParam.KEY_SIZE, svParam.kvParam.VALUE_SIZE}
        ){
            for (int i = 0; i < _bankInterfaces.size(); i++){
                BankInputInterface* bankInItf = _bankInterfaces[i];
                bankInItf->setInputParam(&qMem.getFront("key"),
                                         &qMem.getFront("value"),
                                         &qMem.getFront("isLoad"));
            }
        }

        void flow() override{
            //////// initializing value and indexing
            var queueAvail = ~qMem.isEmpty();
            //////// deal with bank interface
            for (int i = 0; i < _bankInterfaces.size(); i++){
                BankInputInterface* bankInItf = _bankInterfaces[i];
                ////// assign reqToDequeue signal that bank is ready to receive
                reqToDequeue(i) = bankInItf->isReqSuccess();
                ////// assign send the data
                Slice bankSl = {_svParam.kvParam.KEY_SIZE - _svParam.prefixBit,_svParam.kvParam.KEY_SIZE};
                bankInItf->sendOn(queueAvail & ((*qMem.getFront("key").doSlice(bankSl)) == bankInItf->bankId));
            }
            /////// deal with queue
            qMem.initLogic();
            cwhile(true){
                zif (reqToDequeue){
                    qMem.deQueue();
                }
            }
        }
    };

}

#endif //SIMPLEINGRESS_H
