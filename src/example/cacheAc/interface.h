//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_PACKETBASE_H
#define KATHRYN_PACKETBASE_H

#include "kathryn.h"
#include "parameter.h"
#include "example/dataStruct/field/dynamicField.h"


namespace kathryn::cacheServer{


    struct BankInputInterface: SingleHandShakeBase{
        const KV_PARAM& _param;
        ull bankId = 0;
        mReg(isLoad, 1);                 Operable* iIsLoad = nullptr; ////// pure isLoad
        mReg(key   , _param.KEY_SIZE);   Operable* iKey    = nullptr; ////// pure key

        std::vector<Reg*> values;        std::vector<Operable*> iValue; ////// pure value

        explicit BankInputInterface(KV_PARAM& param, int bId):
        _param(param),bankId(bId){
            for (int i = 0; i < _param.valuefield._valueFieldNames.size(); i++){
                values.push_back(&mOprReg(_param.valuefield._valueFieldNames[i], 1));
            }
        }

        void transferPayLoad() override{
            isLoad <<= *iIsLoad;
            key    <<= *iKey;
            /////// check transfer size
            assert(_param.valuefield._valueFieldNames.size() == values.size());
            assert(values.size() == iValue.size());
            for (int i = 0; i < _param.valuefield._valueFieldNames.size(); i++){
                *values[i] <<= *iValue[i];
            }
        }

        void setInputParam(Operable* l, Operable* k, std::vector<Operable*> vs){
            iIsLoad = l; iKey = k; iValue = vs;
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
        std::vector<Operable*> iValues;
        Wire*     readEn      = nullptr;

        mWire(outTest, 1);

        explicit BankOutputInterface(
            const KV_PARAM& param,
            BankInputInterface& inputItf
        ): SingleHandShakeBase(false)
        ,_param(param), _inputItf(inputItf){}

        void setPayLoad(Operable* k, std::vector<Operable*> vs, Wire* e){
            resultKey = k; iValues = vs; readEn = e;
        }

        void transferPayLoad() override{
            (*readEn) = 1;
            zif (isReqSuccess())
                _inputItf.declareReadyToRcv();
        }

    };

}

#endif //KATHRYN_PACKETBASE_H