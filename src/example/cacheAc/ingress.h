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
        Queue qMem;///// | isLoad<1> | key | value
        ////// to do make input egress
        std::vector<BankInputInterface*> _bankInterfaces;

        mWire(reqToDequeue, _bankInterfaces.size());

        IngressBase(
            SERVER_PARAM                     svParam,
            std::vector<BankInputInterface*> bankInterfaces,
            const std::vector<std::string>&  queueMetaNames,
            const std::vector<int>&          queueMetaSize):
        _svParam(svParam),
        qMem(_svParam.ingrQueueSize,
        queueMetaNames,
        queueMetaSize),
        _bankInterfaces(std::move(bankInterfaces)){}

        virtual int  getAvailableBank(){return (int)_bankInterfaces.size();}
    };

}

#endif //KATHRYN_INGRESS_H