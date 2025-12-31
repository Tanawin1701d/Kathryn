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
            KV_PARAM&      _kb_param;
            DYNAMIC_FIELD fields; //// valid bit include
            const int     LIMIT_TIME = 60000;
            const int     AMT_WORD   = 1;
            const int     _bankId    = -1;


            int readCountIdx = 0;
            int writeCountIdx = 0;

            /////////////// list vector
            std::vector<Operable*>     readActivation;
            std::vector<Operable*>     readIndexers;
            std::vector<Operable*>     writeActivation;
            std::vector<Operable*>     writeIndexers;
            std::vector<Operable*>     writeValues;

            ///////////// <valid bit><value>
            mMem(poolData, AMT_WORD, fields.sumFieldSize());
            mReg(timerCnt, 16);

            /** global memory management*/

            /////// for read
            mWire(globReadIndexer, log2Ceil(AMT_WORD));
            mWire(globReadOutput , fields.sumFieldSize());

            /////// for write
            mWire(globWriteEnable , 1);
            mWire(globWriteIndexer, log2Ceil(AMT_WORD));
            mWire(globWriteValue  , fields.sumFieldSize());

            mVal(DUMMY_RESET_VALUE, fields.sumFieldSize(), 0);

            mWire(wa, 1);

            CacheBankBase(KV_PARAM& kv_param, const int amount_word, int bankId):
            _kb_param(kv_param),
            fields   (kv_param.valuefield + DYNAMIC_FIELD({"valid"}, {1})),
            AMT_WORD(amount_word),
            _bankId(bankId){}

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
                BankInputInterface* inItf = getBankInputInterface();

                cwhile(true){
                    cif(timerCnt == LIMIT_TIME){ strack("maintenance" + std::to_string(_bankId))
                        maintenanceBank();
                    }celif(inItf->isNextCycleBusy()){ strack("decode" + std::to_string(_bankId));
                        decodePacket();
                    }celse{
                        wa = 1;
                        ////syWait(1);
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
                        mVal(valid, 1, 1);
                        globWriteValue   = gr(valid, *writeValues[idx]);
                    }
                }
            }

            std::vector<Operable*> getReadValue(){
                std::vector<Operable*> result;
                for (std::string valueName: _kb_param.valuefield._valueFieldNames){
                    int idx      = fields.findIdx(valueName);
                    int startBit = fields.findStartBit(idx);
                    int stopBit  = startBit + fields.getSize(idx);
                    result.push_back(&globReadOutput(startBit, stopBit));
                }
                assert(!result.empty());
                return result;
            }

            std::pair<Wire&, std::vector<Operable*>> readMem(Operable& addr){
                Wire& bit = makeOprWire("readEnSig" + std::to_string(readCountIdx++), 1);
                assert(addr.getOperableSlice().getSize() == log2Ceil(AMT_WORD));
                readActivation.push_back(&bit);
                readIndexers.push_back(&addr);
                return {bit, getReadValue()};
            }

            ////// data [a3, a2, a1, a0]
            Operable& composedDataToAssign(std::vector<Reg*> datas){
                ////// because nest need reverse assumption
                std::reverse(datas.begin(), datas.end());
                std::vector<Operable*> oprs;
                for (Reg* opr: datas){
                    assert(opr != nullptr);
                    oprs.push_back(opr);
                }
                return makeNestManReadOnly(true, oprs);
            }

            void writeMem(Operable& idx, Operable& value){
                Wire& activateWire = makeOprWire("writeEnSig" + std::to_string(writeCountIdx++), 1) = 1;

                assert(idx.getOperableSlice().getSize() == log2Ceil(AMT_WORD));
                writeActivation.push_back(&activateWire);
                writeIndexers  .push_back(&idx);
                writeValues    .push_back(&value);
            }

            void resetMem(Operable& idx){
                writeMem(idx, DUMMY_RESET_VALUE);
            }

            /** this work only for simulation */
            /*** return vector of key and value  ***/
            std::vector<KV_DEBUG> getActiveValueDebug(){

                std::vector<KV_DEBUG> result;
                for (int row = 0; row < AMT_WORD; row++){
                        ///// check the valid bit
                        ///////// this is set to < 64
                        ull  readData = poolData.at(row).getVal();
                        bool valid    = (readData >> _kb_param.valuefield.sumFieldSize());

                        if (valid){
                            std::string key   = std::to_string(row);
                            std::string value = std::to_string(readData & (((ull)1 << _kb_param.valuefield.sumFieldSize())-1) );
                            result.push_back({key, value});
                        }
                }
                return result;
            }
        };
    }

#endif //KATHRYN_BANK_H