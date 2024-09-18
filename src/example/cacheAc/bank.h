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

            int readCountIdx = 0;
            int writeCountIdx = 0;

            /////////////// list vector
            std::vector<Operable*>     readActivation;
            std::vector<Operable*>     readIndexers;
            std::vector<Operable*>     writeActivation;
            std::vector<Operable*>     writeIndexers;
            std::vector<Operable*>     writeValues;

            mMem(poolData, AMT_WORD, _kb_param.VALUE_SIZE + EXTEND_WORD_BIT);
            mReg(timerCnt, 16);

            /** global memory management*/

            /////// for read
            mWire(globReadIndexer, _kb_param.KEY_SIZE);
            mWire(globReadOutput , _kb_param.VALUE_SIZE + EXTEND_WORD_BIT);

            /////// for write
            mWire(globWriteEnable , 1);
            mWire(globWriteIndexer, _kb_param.KEY_SIZE);
            mWire(globWriteValue , _kb_param.VALUE_SIZE + EXTEND_WORD_BIT);


            mVal(DUMMY_RESET_VALUE, _kb_param.VALUE_SIZE + EXTEND_WORD_BIT, 0);



            CacheBankBase(KV_PARAM kv_param, const int amount_word,
                const int extendWordBit):
            _kb_param(kv_param),
            EXTEND_WORD_BIT(extendWordBit),
            AMT_WORD(amount_word){


            }

            ~CacheBankBase() override = default;

            virtual void                 decodePacket()           = 0; ////// retrieve packet from queue do it your own
            virtual void                 maintenanceBank()        = 0; ////// do  maintenance bank
            virtual BankInputInterface*  getBankInputInterface()  = 0; ////// get the bank input interface
            virtual BankOutputInterface* getBankOutputInterface() = 0; ////// get the bank output interface

            void flow() override{
                doTimer();
                doSchedule();
                doShardMem();
            }

            void doTimer(){
                zif(getResetSignal()){
                    timerCnt <<= 0;
                }
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

            void doShardMem(){
                //////// read the data

                globReadOutput = poolData[globReadIndexer];
                for (int idx = 0; idx < readActivation.size(); idx++){
                    zif (*readActivation[idx]){
                        globReadIndexer = *readIndexers[idx];
                    }
                }

                //////// write the data

                zif (globWriteEnable){
                    poolData[globWriteIndexer] <<= globWriteValue;
                }

                globWriteEnable = ~(makeNestManReadOnly(true, writeActivation) == 0);
                for (int idx = 0; idx < writeActivation.size(); idx++){
                    zif (*writeActivation[idx]){
                        globWriteIndexer = *writeIndexers[idx];
                        globWriteValue   = *writeValues[idx];
                    }
                }
            }





            Operable& readMem(Operable& idx){
                Wire& bit = makeOprWire("readEnSig" + std::to_string(readCountIdx++), 1) = 1;
                assert(idx.getOperableSlice().getSize() == _kb_param.KEY_SIZE);
                readActivation.push_back(&bit);
                readIndexers.push_back(&idx);
                return globReadOutput;
            }

            void writeMem(Operable& idx, Operable& value){
                Wire& bit = makeOprWire("writeEnSig" + std::to_string(writeCountIdx++), 1) = 1;
                assert(idx.getOperableSlice().getSize() == _kb_param.KEY_SIZE);
                writeActivation.push_back(&bit);
                writeIndexers.push_back(&idx);
                writeValues.push_back(&value);
            }

            Operable& getValidBit(Operable& idx){
                return *readMem(idx).doSlice({_kb_param.VALUE_SIZE, _kb_param.VALUE_SIZE + 1});
            }
            Operable& getValue(Operable& idx){
                return *readMem(idx).doSlice({0, _kb_param.VALUE_SIZE});
            }

            void resetMem(Operable& idx){
                writeMem(idx, DUMMY_RESET_VALUE);
            }


        };
    }

#endif //KATHRYN_BANK_H
