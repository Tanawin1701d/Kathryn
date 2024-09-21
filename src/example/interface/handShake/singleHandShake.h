//
// Created by tanawin on 21/9/2024.
//

#ifndef src_example_interface_handShake_SINGLEHANDSHAKE_H
#define src_example_interface_handShake_SINGLEHANDSHAKE_H

#include "kathryn.h"

namespace kathryn{

    struct I_SHS{ /////// single hange

        /////// internal variable
        mReg (valid, 1);
        mWire(lastItemFin , 1); /// it mean inside is not ready to rcv new item

        /////// expose
        mWire(reqResult, 1); //// it is output result
        mWire(reqToSend , 1); ///// it is input from other side

        virtual ~I_SHS() = default;

        void buildLogic(){
            reqResult = (~valid | lastItemFin) & reqToSend;
            zif(reqResult){ ////// transfer in
                valid <<= 1;
                transfer();
            }zelif(lastItemFin){
                valid <<= 0;
            }
        }

        virtual void transfer() = 0;

        //////// for sender
        void requestToSend(){
            reqToSend = 1;
        }

        void tellFinish(){
            lastItemFin = 1;
        }

        Operable& isValid(){
            return valid;
        }

        Operable& isReqSuccess(){
            return reqResult;
        }

    };

}

#endif src_example_interface_handShake_SINGLEHANDSHAKE_H
