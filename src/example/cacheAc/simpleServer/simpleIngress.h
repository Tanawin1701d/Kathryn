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
        const int PREFIX_BIT = 0;

        explicit SimpleIngress(
            SERVER_PARAM                svParam,
            std::vector<BankInputInterface*> bankInterfaces):
        IngressBase(svParam, std::move(bankInterfaces)),
        PREFIX_BIT(svParam.prefixBit){
            assert(PREFIX_BIT > 0);
        }


        void diverseToBank() override{
            ///// | isLoad | key | value |
            int keySize = _svParam.kvParam.KEY_SIZE;
            int valueSize = _svParam.kvParam.VALUE_SIZE;

            int key_start_idx = valueSize;
            int isLoad_start_idx = key_start_idx + keySize;

            Operable& value  = inputQueue.getFront()(0, valueSize);
            Operable& key    = inputQueue.getFront()(key_start_idx,key_start_idx + keySize);
            Operable& isLoad = inputQueue.getFront()(isLoad_start_idx, isLoad_start_idx + 1);


            int stopIdent  = keySize;
            int startIdent = keySize - PREFIX_BIT;
            int amtBank    = 1 << PREFIX_BIT;


            for (int idx = 0; idx < amtBank; idx++){
                zif(
                    ((*key.doSlice({startIdent, stopIdent})) == idx)
                     ){
                    _bankInterfaces[idx]->readyToSend = 1;
                    zif(_bankInterfaces[idx]->readyToRcv){
                        _bankInterfaces[idx]->key    <<= key;
                        _bankInterfaces[idx]->isLoad <<= isLoad;
                        zif(~isLoad){
                            _bankInterfaces[idx]->value <<= value;
                        }
                        inputQueue.deQueue();
                    }
                }
            }

        }

    };

}

#endif //SIMPLEINGRESS_H
