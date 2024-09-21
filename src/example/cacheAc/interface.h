//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_PACKETBASE_H
#define KATHRYN_PACKETBASE_H

#include "kathryn.h"
#include "parameter.h"


    namespace kathryn::cacheServer{

        struct BankInputInterface{
            const KV_PARAM& _param;

            /** register  parameter*/
            mReg (key   , _param.KEY_SIZE); ////// pure key
            mReg (value , _param.VALUE_SIZE);
            mReg (isLoad, 1);

            /** io parameter*/
            mWire(readyToRcv  , 1); ////// cache bank specify that we are ready to rcv
            mWire(readyToSend , 1); ////// ingress desire that they are ready to send
            mWire(requestKey  , _param.KEY_SIZE);
            mWire(requestValue, _param.VALUE_SIZE);
            mWire(requestMode , 1); ////// 1 is load else write

            /////// constructor
            explicit BankInputInterface(const KV_PARAM& param):
            _param(param){}
            virtual ~BankInputInterface() = default;

            void tryRecv(){
                zif (readyToSend){
                    readyToRcv = 1;
                    key    <<= requestKey;
                    value  <<= requestValue;
                    isLoad <<= requestMode;
                }
            }

            Operable& isRcv(){
                return readyToRcv;
            }
        };

        struct BankOutputInterface{
            const KV_PARAM& _param;
            mWire(resultKey  , _param.KEY_SIZE);
            mWire(resultValue, _param.VALUE_SIZE);
            mWire(readyToRcv , 1); ////// out gress specify
            mWire(readyToSend, 1); ////// bank specify

            explicit BankOutputInterface(const KV_PARAM& param):
            _param(param){}

            void forceSend(Operable& key, Operable& value){
                cdowhile(~readyToRcv){
                    resultKey = key;
                    resultValue = value;
                    readyToSend = 1;
                }
            }
        };

    }



#endif //KATHRYN_PACKETBASE_H
