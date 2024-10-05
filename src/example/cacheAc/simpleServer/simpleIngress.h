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
            SERVER_PARAM&                    svParam,
            std::vector<BankInputInterface*> bankInterfaces):
        IngressBase(svParam, std::move(bankInterfaces),
        svParam.kvParam.valuefield +
        DYNAMIC_FIELD({"key", "isLoad"}, {svParam.kvParam.KEY_SIZE, 1})
        ){
            for (auto bankInItf : _bankInterfaces){
                bankInItf->setInputParam(&_qMem.getFront("isLoad"),
                                         &_qMem.getFront("key"),
                                         _qMem.getFront(svParam.kvParam.valuefield._valueFieldNames)
                                         );
            }
        }

        void flow() override{
            //////// initializing value and indexing
            var queueAvail = ~_qMem.isEmpty();
            //////// deal with bank interface
            for (int i = 0; i < _bankInterfaces.size(); i++){
                BankInputInterface* bankInItf = _bankInterfaces[i];
                ////// assign reqToDequeue signal that bank is ready to receive
                reqToDequeue(i) = bankInItf->isReqSuccess();
                ////// assign send the data
                Slice bankSl = {_svParam.kvParam.KEY_SIZE - _svParam.prefixBit,_svParam.kvParam.KEY_SIZE};
                bankInItf->sendOn(queueAvail & ((*_qMem.getFront("key").doSlice(bankSl)) == bankInItf->bankId));
            }
            /////// deal with queue
            _qMem.initLogic();
            cwhile(true){
                zif (reqToDequeue){
                    _qMem.deQueue();
                }
            }
        }
    };

}

#endif //SIMPLEINGRESS_H
