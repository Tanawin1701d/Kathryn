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
                result.push_back("bankIdx_" + std::to_string(bankIdx));
            }
            return result;
        }

        //////////////////// record base
        void recordSlot(){
            recordIngress();
            recordOutgress();
            recordBank();
            iterateCycle();

        }
        /////////////////// generate ingress block
        void recordIngress(){
            ///SimpleIngress& ingr = *((SimpleIngress*)_simpleServer._ingress);
            SERVER_PARAM svParam = _simpleServer._svParam;
            Queue& ingressQueue = _simpleServer._ingress->inputQueue;
            auto queueDebugValue=
                ingressQueue.getSimDebug(
                        {1, svParam.kvParam.KEY_SIZE, svParam.kvParam.VALUE_SIZE}
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

            if ( ((ull)outgr.oValid) == 0 ){
                addSlotVal(OUTR_SLOT_IDX, "nop");
                return;
            }

            addSlotVal(OUTR_SLOT_IDX, "k " + std::to_string((ull)outgr.oKey));
            addSlotVal(OUTR_SLOT_IDX, "v " + std::to_string((ull)outgr.oValue));

        }

        void recordBank(){

            std::vector<CacheBankBase*> cacheBanks = _simpleServer.getRefBanks();
            for (int bankIdx = 0; bankIdx < cacheBanks.size(); bankIdx++){
                assert(cacheBanks[bankIdx] != nullptr);
                auto filledElement = cacheBanks[bankIdx]->getActiveValueDebug();
                for (auto[key, val]: filledElement){
                    addSlotVal(BANK_SLOT_START_IDX + bankIdx, "k " + key + " v " + val);
                }
            }

        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_CACHESLOTWRITER_H
