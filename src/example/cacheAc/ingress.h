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
        Queue inputQueue;
        ////// to do make input egress
        std::vector<BankInputInterface*> _bankInterfaces;
        ///// it is package on each bank for control

        IngressBase(
            SERVER_PARAM                svParam,
            std::vector<BankInputInterface*> bankInterfaces):
        _svParam(svParam),
        inputQueue(_svParam.kvParam.KEY_SIZE + 1, _svParam.ingrQueueSize),
        _bankInterfaces(std::move(bankInterfaces)){}

        void flow() override{
            ////////// for put data to bank
            cwhile (true){
                /////// for now we assume we have packet queue
                cif (inputQueue.isEmpty()){
                    syWait(1);
                }celse{
                    diverseToBank();
                }
            }
        }

        virtual void diverseToBank() = 0;
        virtual int  getAvailableBank(){return (int)_bankInterfaces.size();}
    };

}

#endif //KATHRYN_INGRESS_H
