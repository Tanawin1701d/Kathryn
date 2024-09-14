//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_PACKETBASE_H
#define KATHRYN_PACKETBASE_H

#include "kathryn.h"
#include "parameter.h"


    namespace kathryn::cacheServer{

        struct BankInterface{
            const KV_PARAM& _param;
            mReg(key   , _param.KEY_SIZE);
            mReg(value , _param.VALUE_SIZE);
            mReg(isLoad, 1);
            /////// constructor
            explicit BankInterface(const KV_PARAM& param):
            _param(param){}
            BankInterface(const BankInterface& pb):
            _param(pb._param){}
            virtual ~BankInterface() = default;
            ////////  data and  meta data retrieve
            virtual int getKeyBitWidth()  {return _param.KEY_SIZE;}
            virtual int getValueBitWidth(){return _param.VALUE_SIZE;}
        };

        // struct IngressInterface{
        //     const KV_PARAM& _param;
        //     mReg(key, _param.KEY_SIZE);
        //     mReg(isLoad, 1);
        //
        //     IngressInterface(const KV_PARAM& param):
        //     _param(param){}
        //
        //     IngressInterface(const IngressInterface& ifa):
        //     _param(ifa._param){}
        //     virtual ~IngressInterface() = default;
        // };

    }



#endif //KATHRYN_PACKETBASE_H
