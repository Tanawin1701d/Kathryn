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

        explicit SimpleBank(KV_PARAM& kv_param, int suffixBit, int bankId):
        CacheBankBase (kv_param, 1 << suffixBit),
        _suffixBit    (suffixBit),
        _bankId       (bankId),
        inputItf      (kv_param, _bankId),
        outputItf     (kv_param, inputItf),
        inputInBankKey(inputItf.key(0, _suffixBit))
        {   ////// build size of
            assert(suffixBit > 0);
            assert(bankId < (1 << suffixBit));
            ////// build Read Structure

        }

        //// Both decodePacket and maintenance bank are created in flow stage
        void decodePacket() override{

            cif (inputItf.nextIsLoad()){ ////// is load /////try until outgress is recv
                auto [enValue, readValues] = readMem(inputInBankKey);
                outputItf.setPayLoad(&inputItf.key, readValues, &enValue);
                outputItf.sendAndWaitUntillSuccess();
            }celse{ //// is write
                ///// write to memory now
                if (_kb_param.replacePol == OVER_WRITE){
                    writeMem(inputInBankKey,
                             composedDataToAssign(inputItf.values));
                }else{ ////// avoid conflict policy
//                    zif(getValidBit(inputInBankKey)){
//                        writeMem(inputInBankKey, inputItf.value);
                    writeMem(inputInBankKey, composedDataToAssign(inputItf.values));
                }

                isWriting = 1;
                inputItf.declareReadyToRcv();
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

        BankInputInterface*  getBankInputInterface () override{return &inputItf;}
        BankOutputInterface* getBankOutputInterface() override{return &outputItf;}

    };

}

#endif //SIMPLEBANK_H