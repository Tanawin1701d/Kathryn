//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_INGRESS_H
#define KATHRYN_INGRESS_H

#include <utility>

#include "kathryn.h"
#include "interface.h"
#include "example/dataStruct/queue/queue.h"



namespace kathryn::cacheServer{

    class IngressBase: public Module{
    public:
        const SERVER_PARAM _svParam;
        ////////////////////////////////////////////
        Queue inputQueue;///// | isLoad<1> | key | value
        ////// to do make input egress
        std::vector<BankInputInterface*> _bankInterfaces;

        mWire(reqToDequeue, _bankInterfaces.size());

        IngressBase(
            SERVER_PARAM                svParam,
            std::vector<BankInputInterface*> bankInterfaces):
        _svParam(svParam),
        inputQueue(
            1 + _svParam.kvParam.KEY_SIZE + _svParam.kvParam.VALUE_SIZE,
            _svParam.ingrQueueSize),
        _bankInterfaces(std::move(bankInterfaces)){}


        virtual int  getAvailableBank(){return (int)_bankInterfaces.size();}
    };

}

#endif //KATHRYN_INGRESS_H