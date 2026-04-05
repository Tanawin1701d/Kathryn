//
// Created by tanawin on 19/9/2024.
//

#ifndef KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_CACHESLOTWRITER_H
#define KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_CACHESLOTWRITER_H

#include "util/fileWriter/slotWriter/slotWriter.h"
#include "example/cacheAc/simpleServer/simpleSystem.h"

namespace kathryn::cacheServer{


    constexpr int INGR_SLOT_IDX = 0;
    constexpr int OUTR_SLOT_IDX = 1;
    constexpr int BANK_SLOT_START_IDX = 2;

    class CacheSlotWriter: public SlotWriter{

        SimpleServer& _simpleServer;

    public:

        CacheSlotWriter(
            SimpleServer& simpleServer,
            std::string fileName
        ):
        SlotWriter(genColumn(simpleServer), 25, fileName),
        _simpleServer(simpleServer){

        }

        //////////////////// generate column name
        std::vector<std::string> genColumn(SimpleServer& baseServer){

            std::vector<std::string> result = { "ingress", "outgress"};
            for (int bankIdx = 0;
                     bankIdx < baseServer.getRefBanks().size();
                     bankIdx++){
                result.pushBack("bankIdx_" + std::toString(bankIdx));
            }
            return result;
        }

        //////////////////// record base
        void recordSlot(){
            recordIngress();
            recordOutgress();
            recordBank();
            concludeEachCycle();

        }
        /////////////////// generate ingress block
        void recordIngress(){
            ///SimpleIngress& ingr = *((SimpleIngress*)_simpleServer._ingress);
            ////////////////////////////// record test

            ////////////////////////////////////////////////////////////////////////////////
            SERVER_PARAM svParam = _simpleServer._svParam;
            Queue& ingressQueue = _simpleServer._ingress->_qMem;

            addSlotVal(INGR_SLOT_IDX, "reqResBank0 " + std::toString((ull)_simpleServer._ingress->_bankInterfaces[0]->reqResult));
            addSlotVal(INGR_SLOT_IDX, "reqResBank1 " + std::toString((ull)_simpleServer._ingress->_bankInterfaces[1]->reqResult));

            addSlotVal(INGR_SLOT_IDX, "reqToDeq " + std::toString((ull)_simpleServer._ingress->reqToDequeue));
            addSlotVal(INGR_SLOT_IDX, "deqIntd " + std::toString((ull)ingressQueue.deqIntend));
            addSlotVal(INGR_SLOT_IDX, "headPos " + std::toString((ull)ingressQueue.headPos));
            addSlotVal(INGR_SLOT_IDX, "lastPos " + std::toString((ull)ingressQueue.lastPos));
            addSlotVal(INGR_SLOT_IDX, "curSize " + std::toString((ull)ingressQueue.curSize));

            auto queueDebugValue=
                ingressQueue.getSimDebug(
                        {1, svParam.kvParam.KEY_SIZE, svParam.kvParam.valuefield.sumFieldSize()}
                );

            for (auto&  queueEle: queueDebugValue){
                addSlotVal(INGR_SLOT_IDX,
               "m " + queueEle[2] +
                " k " + queueEle[1] +
                " v " + queueEle[0]
                );
            }
        }

        void recordOutgress(){
            SimpleOutgress& outgr = *((SimpleOutgress*)_simpleServer._outgress);

            addSlotVal(OUTR_SLOT_IDX, "areThereFin " + std::toString((ull)outgr.areThereFin));
            addSlotVal(OUTR_SLOT_IDX, "curBankIdx " + std::toString((ull)outgr.curBankItr));

            if ( ((ull)outgr.oValid) == 0 ){
                addSlotVal(OUTR_SLOT_IDX, "nop");
                return;
            }

            addSlotVal(OUTR_SLOT_IDX, "k " + std::toString((ull)outgr.oKey));

            int idx = 0;
            for (Reg* reg: outgr.oValues){
                addSlotVal(OUTR_SLOT_IDX,
                    "v" + std::toString(idx) + " " + std::toString((ull)*reg));
            }

        }

        void recordBank(){

            std::vector<CacheBankBase*> cacheBanks = _simpleServer.getRefBanks();
            for (int bankIdx = 0; bankIdx < cacheBanks.size(); bankIdx++){
                assert(cacheBanks[bankIdx] != nullptr);
                auto filledElement = cacheBanks[bankIdx]->getActiveValueDebug();

                BankInputInterface&  inputInterface   = ((SimpleBank*)cacheBanks[bankIdx])->inputItf;
                BankOutputInterface& outputInterface  = ((SimpleBank*)cacheBanks[bankIdx])->outputItf;
                SimpleBank&          simpleBank       = *(SimpleBank*)cacheBanks[bankIdx];

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "valid " +
                           std::toString((ull)inputInterface.isBusy));
                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "value " +
                           std::toString((ull)(*inputInterface.values[0])));
                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "key " +
                           std::toString((ull)inputInterface.key));
                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "mode " +
                           std::toString((ull)inputInterface.isLoad));
                //-----------------------------------------------------------------------------------

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "timer " +
                           std::toString((ull)(((SimpleBank*)cacheBanks[bankIdx])->timerCnt)));
                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "isWriting " +
                           std::toString((ull)(((SimpleBank*)cacheBanks[bankIdx])->isWriting)));
                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "cleanCnt " +
                           std::toString((ull)(((SimpleBank*)cacheBanks[bankIdx])->cleanCnt)));
                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "outStuck " +
                           std::toString((ull)(((SimpleBank*)cacheBanks[bankIdx])->outputItf.outTest)));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "waitCycle " +
                           std::toString((ull)(((SimpleBank*)cacheBanks[bankIdx])->wa)));
                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "isLoad " +
                           std::toString((ull)inputInterface.isLoad));
                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "ReqResult " +
                           std::toString((ull)inputInterface.reqResult));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "isReqToSend " +
                           std::toString((ull)inputInterface.isReqToSend()));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "isValid " +
                           std::toString((ull)inputInterface.isBusy));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "lasatItemFin " +
                           std::toString((ull)inputInterface.readyToGetNew));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "------- read Result");

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "resultKey" + std::toString((ull)outputInterface.resultKey));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "resultValue" + std::toString((ull)(*outputInterface.iValues[0])));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                                "ReqResult " + std::toString((ull)outputInterface.reqResult));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "isReqToSend " +
                           std::toString((ull)outputInterface.isReqToSend()));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "isValid " +
                           std::toString((ull)outputInterface.isBusy));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "lasatItemFin " +
                           std::toString((ull)outputInterface.readyToGetNew));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "globReadIdx " +
                           std::toString((ull)simpleBank.globReadIndexer));

                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "readActivation " +
                           std::toString((ull)(*simpleBank.readActivation[0])));
                addSlotVal(BANK_SLOT_START_IDX + bankIdx,
                           "readValue " +
                           std::toString((ull)(*outputInterface.iValues[0])));


                for (auto[key, val]: filledElement){
                    addSlotVal(BANK_SLOT_START_IDX + bankIdx, "k " + key + " v " + val);

                }
            }

        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_CACHESLOTWRITER_H
