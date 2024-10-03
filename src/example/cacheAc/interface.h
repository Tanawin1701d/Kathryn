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

        ull bankId = 0;

        explicit BankInputInterface(KV_PARAM& param, int bId):
        _param(param),
        bankId(bId){}

        ~BankInputInterface(){delete _inputParam;}

        void transfer() override{
            assert(_inputParam != nullptr);
            key    <<= _inputParam->iKey;
            value  <<= _inputParam->iValue;
            isLoad <<= _inputParam->iIsLoad;
        }

        void setInputParam(INPUT_PARAM inputParam){
            _inputParam = new INPUT_PARAM(inputParam);
        }

        Operable& nextIsLoad(){
            //////// we need to make sure that decision is correct for next cycle
            return (reqResult & _inputParam->iIsLoad) | (~reqResult & valid & isLoad);
        }

        ////// for get bank key


    };

    struct BankOutputInterface: I_SHS{
        const KV_PARAM& _param;
        mWire(resultKey  , _param.KEY_SIZE);
        mWire(resultValue, _param.VALUE_SIZE);

        mWire(outTest, 1);

        explicit BankOutputInterface(const KV_PARAM& param):
            _param(param){}

        //////// acknowledge Signal used to tell that output is finish sent
        //////// this function should be used as parallel
        void forceSend(BankInputInterface& inputItf,
                       Operable& key,
                       Operable& value,
                       Wire&     readEn

        ){
            cdowhile(~isReqSuccess()){
                requestToSend();
                resultKey   = key;
                resultValue = value;
                readEn      = 1;
                outTest     = 1;
                zif(isReqSuccess()){
                    inputItf.tellFinish();
                }
            }
        }

        void transfer() override{}

    };

}

#endif //KATHRYN_PACKETBASE_H