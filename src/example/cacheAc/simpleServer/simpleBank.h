//
// Created by tanawin on 14/9/2024.
//

#ifndef SIMPLEBANK_H
#define SIMPLEBANK_H


#include "../bank.h"

namespace kathryn::cacheServer{

    class SimpleBank: public CacheBankBase{
    public:
        const int _suffixBit = -1;
        BankInputInterface inputItf;
        BankOutputInterface outputItf;
        mReg(cleanCnt, _suffixBit);

        //////////////////////////////////////////////////////////
        ///   | valid_bit | data_bit |
        //////////////////////////////////////////////////////////

        explicit SimpleBank(KV_PARAM kv_param, int suffixBit):
        CacheBankBase(kv_param, 1 << suffixBit, 1),
        _suffixBit(suffixBit),
        inputItf(kv_param),
        outputItf(kv_param)
        {
            ////// build size of
            assert(suffixBit > 0);
        }

        void decodePacket() override{

            mVal(validBitAss, 1,1);

            seq{
                //////// one cycle
                zif (inputItf.readyToSend){
                    inputItf.readyToRcv = 1;
                }
                //////// next cycle
                cif(inputItf.readyToSend){
                    par{
                        zif (inputItf.isLoad){
                            //// TODO next
                        }zelse{
                            //// TODO next
                        }
                    }
                }
            }
        }

        void maintenanceBank() override{
            seq{
                cleanCnt <<= 0;
                cdowhile(cleanCnt != (1 << _suffixBit - 1)){
                    par{
                        cleanCnt <<= cleanCnt + 1;
                        writeMem(cleanCnt, 0);
                    }
                }
            }
        }

        BankInputInterface* getBankInputInterface() override{
            return &inputItf;
        }

        BankOutputInterface* getBankOutputInterface() override{
            return &outputItf;
        }








    };

}

#endif //SIMPLEBANK_H
