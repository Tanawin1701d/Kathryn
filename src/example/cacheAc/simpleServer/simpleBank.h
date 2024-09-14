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
        BankInterface smBankInterface;
        mReg(cleanCnt, _suffixBit);

        //////////////////////////////////////////////////////////
        ///   | valid_bit | data_bit |
        //////////////////////////////////////////////////////////

        explicit SimpleBank(KV_PARAM kv_param, int suffixBit):
        _suffixBit(suffixBit),
        CacheBankBase(kv_param, 1 << suffixBit, 1),
        smBankInterface(kv_param)
        {
            ////// build size of
            assert(suffixBit > 0);
        }

        void decodePacket() override{

            mVal(validBitAss, 1,1);

            cif(smBankInterface.isLoad){
                /** put to outgress*/
            }celse{
                writeMem(
                    smBankInterface.key(0, smBankInterface.getKeyBitWidth()-_suffixBit),
                    g(validBitAss, smBankInterface.value)
                );
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

        BankInterface* getBankInterface() override{
            return &smBankInterface;
        }








    };

}

#endif //SIMPLEBANK_H
