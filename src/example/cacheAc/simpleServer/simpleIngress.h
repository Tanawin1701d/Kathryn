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
        mWire(isSendable, 1);

        explicit SimpleIngress(
            SERVER_PARAM                svParam,
            std::vector<BankInputInterface*> bankInterfaces,
            const int                   prefixBit
        ):IngressBase(svParam, std::move(bankInterfaces)),
          PREFIX_BIT(prefixBit){
            assert(PREFIX_BIT > 0);
        }


        void diverseToBank() override{
            int qWordSz = inputQueue.WORD_SZ;
            Operable& key    = inputQueue.getFront()(0, qWordSz-1);
            Operable& isLoad = inputQueue.getFront()(qWordSz-1);

            int keySize    = key.getOperableSlice().getSize();
            int stopIdent  = keySize;
            int startIdent = keySize - PREFIX_BIT;
            int amtBank    = 1 << PREFIX_BIT;

            zif (((*key.doSlice({startIdent, stopIdent})) == 0) &
                     (_bankInterfaces[0]->isFree)){
                isSendable = 1;
                _bankInterfaces[0]->key <<= key;
                _bankInterfaces[0]->isLoad <<= isLoad;

            }

            for (int idx = 1; idx < amtBank; idx++){
                zelif(
                    ((*key.doSlice({startIdent, stopIdent})) == idx) &
                     (_bankInterfaces[idx]->isFree)
                     ){
                    isSendable = 1;
                    _bankInterfaces[idx]->key <<= key;
                    _bankInterfaces[idx]->isLoad <<= isLoad;
                }
            }

        }

    };

}

#endif //SIMPLEINGRESS_H
