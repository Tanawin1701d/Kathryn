//
// Created by tanawin on 11/9/2024.
//

#ifndef KATHRYN_BANK_H
#define KATHRYN_BANK_H

#include "kathryn.h"
#include  "interface.h"


    namespace kathryn::cacheServer{

        class CacheBankBase: public Module{
        public:
            const KV_PARAM _kb_param;
            const int EXTEND_WORD_BIT = 1;
            const int LIMIT_TIME = 60000;
            const int AMT_WORD   = 1;

            mMem(poolData, AMT_WORD, _bankInterface.getValueBitWidth());
            mReg(timerCnt, 16);


            CacheBankBase(KV_PARAM kv_param, const int amount_word,
                const int extendWordBit):
            _kb_param(kv_param),
            EXTEND_WORD_BIT(extendWordBit),
            AMT_WORD(amount_word){}

            ~CacheBankBase() override = default;

            virtual void           decodePacket()    = 0;
            virtual void           maintenanceBank() = 0;
            virtual BankInterface* getBankInterface() = 0;

            void flow() override{
                doTimer();
                doSchedule();
            }

            void doTimer(){
                cwhile(true){
                    zif(timerCnt == LIMIT_TIME) timerCnt <<= 0;
                    zelse                       timerCnt <<= timerCnt + 1;
                }
            }

            void doSchedule(){
                cwhile(true){
                    cif(timerCnt == LIMIT_TIME){
                        maintenanceBank();
                    }celse{
                        decodePacket();
                    }
                }
            }

            Operable& readMem(Operable& idx){
                ///// todo do this
            }

            void writeMem(Operable& idx, Operable& value){

            }

            void writeMem(Operable& idx, ull value){

            }


        };
    }

#endif //KATHRYN_BANK_H
