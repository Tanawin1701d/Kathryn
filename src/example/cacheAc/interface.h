//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_PACKETBASE_H
#define KATHRYN_PACKETBASE_H

#include "kathryn.h"
#include "parameter.h"
#include "example/interface/handShake/singleHandShake.h"


namespace kathryn::cacheServer{

    struct INPUT_PARAM{
        Operable& iKey;
        Operable& iValue;
        Operable& iIsLoad;
    };

    struct BankInputInterface: I_SHS{

        const KV_PARAM& _param;
        INPUT_PARAM*    _inputParam = nullptr;

        mReg(key   , _param.KEY_SIZE);   ////// pure key
        mReg(value , _param.VALUE_SIZE); ////// pure value
        mReg(isLoad, 1);                 ////// pure isLoad

        BankInputInterface(KV_PARAM& param):
        _param(param){}

        void transfer() override{
            key    <<= _inputParam->iKey;
            value  <<= _inputParam->iValue;
            isLoad <<= _inputParam->iIsLoad;
        }

        void setInputParam(INPUT_PARAM& inputParam){
            _inputParam = &inputParam;
        }

    };

    struct BankOutputInterface: I_SHS{
        const KV_PARAM& _param;
        mWire(resultKey  , _param.KEY_SIZE);
        mWire(resultValue, _param.VALUE_SIZE);

        explicit BankOutputInterface(const KV_PARAM& param):
            _param(param){}

        //////// acknowledge Signal used to tell that output is finish sent
        //////// this function should be used as parallel
        void forceSend(BankInputInterface& inputItf,
                       Operable& key,
                       Operable& value

        ){

            cdowhile(~isReqSuccess()){
                requestToSend();
                resultKey = key;
                resultValue = value;
                zif(isReqSuccess()){
                    inputItf.tellFinish();
                }
            }

        }

        void transfer() override{}

    };

}

#endif //KATHRYN_PACKETBASE_H
