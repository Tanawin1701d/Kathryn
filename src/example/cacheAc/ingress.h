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
        Queue qMem;// TYPICALLY  /// | isLoad<1> | key | value
        ////// to do make input egress
        std::vector<BankInputInterface*> _bankInterfaces;

        mWire(reqToDequeue, _bankInterfaces.size());

        IngressBase(
            SERVER_PARAM                     svParam,
            std::vector<BankInputInterface*> bankInterfaces,
            DYNAMIC_FIELD                    dyField):
        _svParam       (svParam),
        qMem           (_svParam.ingrQueueSize,dyField),
        _bankInterfaces(bankInterfaces){}

        virtual int  getAvailableBank(){return (int)_bankInterfaces.size();}
    };

}

#endif //KATHRYN_INGRESS_H