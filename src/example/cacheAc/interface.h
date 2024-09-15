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
            mReg (key   , _param.KEY_SIZE); ////// pure key
            mReg (isLoad, 1);               ////// is loaded or not
            mWire(isFree, 1);               ////// is free bank ---> specify It
            /////// constructor
            explicit BankInputInterface(const KV_PARAM& param):
            _param(param){}
            BankInputInterface(const BankInputInterface& pb):
            _param(pb._param){}
            virtual ~BankInputInterface() = default;
            ////////  data and  meta data retrieve
            virtual int getKeyBitWidth()  {return _param.KEY_SIZE;}
            virtual int getValueBitWidth(){return _param.VALUE_SIZE;}
        };

        struct BankOutputInterface{
            const KV_PARAM& _param;
            mWire(resultKey  , _param.KEY_SIZE);
            mWire(resultValue, _param.VALUE_SIZE);
            mWire(readyToRcv , 1); ////// out gress specify
            mWire(readyToSend, 1); ////// bank specify

            explicit BankOutputInterface(const KV_PARAM& param):
            _param(param){}







        };


    }



#endif //KATHRYN_PACKETBASE_H
