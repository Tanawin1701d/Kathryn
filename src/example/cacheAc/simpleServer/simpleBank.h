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
                //////// first cycle
                par{inputItf.tryRecv();}  //////// take one cycle
                //////// next cycle
                cif(inputItf.isRcv()){
                    par{
                        cif (inputItf.isLoad){ ////// is load /////try until outgress is recv
                            outputItf.forceSend(inputItf.key, getValue(inputItf.key));
                        }celse{ //// is write
                            ///// write to memory now
                            if (_kb_param.replacePol == OVER_WRITE) {
                                writeMem(inputItf.key, inputItf.value);
                            }else{ ////// avoid conflict policy
                                zif(getValidBit(inputItf.key)){
                                    writeMem(inputItf.key, inputItf.value);
                                }
                            }
                        }
                    }
                }
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

        BankInputInterface* getBankInputInterface() override{
            return &inputItf;
        }

        BankOutputInterface* getBankOutputInterface() override{
            return &outputItf;
        }








    };

}

#endif //SIMPLEBANK_H
