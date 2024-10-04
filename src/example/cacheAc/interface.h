//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_PACKETBASE_H
#define KATHRYN_PACKETBASE_H

#include "kathryn.h"
#include "parameter.h"


namespace kathryn::cacheServer{


    struct BankInputInterface: SingleHandShakeBase{
        const KV_PARAM& _param;
        ull bankId = 0;
        mReg(key   , _param.KEY_SIZE);   Operable* iKey    = nullptr; ////// pure key
        mReg(value , _param.VALUE_SIZE); Operable* iValue  = nullptr; ////// pure value
        mReg(isLoad, 1);                 Operable* iIsLoad = nullptr; ////// pure isLoad

        explicit BankInputInterface(KV_PARAM& param, int bId):
        _param(param),bankId(bId){}

        void transferPayLoad() override{
            key    <<= *iKey;
            value  <<= *iValue;
            isLoad <<= *iIsLoad;
        }

        void setInputParam(Operable* k, Operable* v, Operable* l){
            iKey = k; iValue = v; iIsLoad = l;
        }

        Operable& nextIsLoad(){
            //////// we need to make sure that decision is correct for next cycle
            return (reqResult & *iIsLoad) | (~reqResult & isCurCycleBusy() & isLoad);
        }

        ////// for get bank key
        ull getBankId(){return bankId;}


    };

    struct BankOutputInterface: SingleHandShakeBase{
        const KV_PARAM& _param;
        BankInputInterface& _inputItf;
        Operable* resultKey   = nullptr;
        Operable* resultValue = nullptr;
        Wire*     readEn      = nullptr;

        mWire(outTest, 1);

        explicit BankOutputInterface(
            const KV_PARAM& param,
            BankInputInterface& inputItf
        ): SingleHandShakeBase(false)
        ,_param(param), _inputItf(inputItf){}

        void setPayLoad(Operable* k, Operable* v, Wire* e){
            resultKey = k; resultValue = v; readEn = e;
        }


        void transferPayLoad() override{
            (*readEn) = 1;
            zif (isReqSuccess())
                _inputItf.declareReadyToRcv();
        }

    };

}

#endif //KATHRYN_PACKETBASE_H