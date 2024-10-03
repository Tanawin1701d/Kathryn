//
// Created by tanawin on 14/9/2024.
//

#ifndef SIMPLEBANK_H
#define SIMPLEBANK_H


#include "../bank.h"

namespace kathryn::cacheServer{

    class SimpleBank: public CacheBankBase{
    public:
        const int           _suffixBit = -1; ///// size of suffix bit
        const int           _bankId    = -1;
        BankInputInterface  inputItf;
        BankOutputInterface outputItf;
        Operable&           inputInBankKey; //// only key that used to indicate bank
        mReg(cleanCnt  , _suffixBit);
        mWire(isWriting, 1         );
        //////////////////////////////////////////////////////////
        ///   | valid_bit | data_bit |
        //////////////////////////////////////////////////////////

        explicit SimpleBank(KV_PARAM kv_param, int suffixBit, int bankId):
        CacheBankBase (kv_param, 1 << suffixBit, 1),
        _suffixBit    (suffixBit),
        _bankId       (bankId),
        inputItf      (kv_param, _bankId),
        outputItf     (kv_param),
        inputInBankKey(inputItf.key(0, _suffixBit))
        {   ////// build size of
            assert(suffixBit > 0);
            assert(bankId < (1 << suffixBit));
        }

        //// Both decodePacket and maintenance bank are created in flow stage
        void decodePacket() override{

            auto [enValue, readValue] = getValue(inputInBankKey);


            std::cout << "packet is decoding" << std::endl;

            cif (inputItf.nextIsLoad()){ ////// is load /////try until outgress is recv
                outputItf.forceSend(
                    inputItf,
                    inputItf.key,
                    readValue,
                    enValue);
            }celse{ //// is write
                ///// write to memory now
                if (_kb_param.replacePol == OVER_WRITE){
                    writeMem(inputInBankKey, inputItf.value);
                }else{ ////// avoid conflict policy
//                    zif(getValidBit(inputInBankKey)){
//                        writeMem(inputInBankKey, inputItf.value);
//                    }
                    writeMem(inputInBankKey, inputItf.value);
                }

                isWriting = 1;
                inputItf.tellFinish();
            }

        }

        void maintenanceBank() override{
            seq{
                cleanCnt <<= 0;
                cdowhile(cleanCnt != ( (1 << _suffixBit) - 1)){
                    par{
                        cleanCnt <<= cleanCnt + 1;
                        resetMem(cleanCnt);
                    }
                }
            }
        }

        void initInterface() override{
            inputItf .buildLogic();
            outputItf.buildLogic();
        }

        BankInputInterface*  getBankInputInterface () override{return &inputItf;}
        BankOutputInterface* getBankOutputInterface() override{return &outputItf;}

    };

}

#endif //SIMPLEBANK_H
