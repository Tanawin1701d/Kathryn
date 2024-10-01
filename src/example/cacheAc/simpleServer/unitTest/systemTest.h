//
// Created by tanawin on 19/9/2024.
//

#ifndef KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_SYSTEMTEST_H
#define KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_SYSTEMTEST_H

#include "kathryn.h"
#include "example/cacheAc/simpleServer/simpleSystem.h"
#include "cacheSlotWriter.h"

namespace kathryn::cacheServer{


    constexpr char VCD_FILE_PARAM        [] = "vcdFile" ;
    constexpr char PROF_FILE_PARAM       [] = "profFile";
    constexpr char CACHE_SLOT_FILE_PARAM [] = "slotFile";

    class CacheSimItf: public SimInterface{

        SimpleServer&   _server;
        CacheSlotWriter _cacheSlotWriter;


    public:

        CacheSimItf(PARAM& params, SimpleServer& server):
        SimInterface(100000,
                     params[VCD_FILE_PARAM],
                     params[PROF_FILE_PARAM],
                     "cacheModel"
                     ),
        _server(server),
        _cacheSlotWriter(_server, params[CACHE_SLOT_FILE_PARAM])
        {}

        //////void describeDef() override{}

        void describe() override{
            /////////// we must prevent this because the queue meta data will be reset
            incCycle(1);
            //////////////   provide data to memory

            sim {
                //// get queue
                Queue &queue = _server.getIngress().inputQueue;
                //// create meta data
                int BANK_AMT = 1 << _server._svParam.prefixBit;
                int AMT_PER_BANK = 1 << (_server._svParam.kvParam.KEY_SIZE - _server._svParam.prefixBit);
                ///// push data to the queue
                for (int idx = 0; idx < queue.WORD_AMT; idx++) {
                    queue.pushDataSim(
                            genIncomePacket(
                                    idx % BANK_AMT,
                                    idx % AMT_PER_BANK,
                                    idx,
                                    false) ///// for now we set all element to write
                    );
                }
            };
        }

        void describeCon() override{
            //////////////   record slot
            for (int cycle = 1; cycle < 90000; cycle++){
                _cacheSlotWriter.recordSlot();
                conNextCycle(1);
            }
        }



        ull genIncomePacket(int bankIdx, int idxInBank, int value, bool isLoad){
            ull baseElement = 0;

            int valueSize   = _server._svParam.kvParam.VALUE_SIZE;
            int keySize     = _server._svParam.kvParam.KEY_SIZE;
            int bankKeySize = _server._svParam.kvParam.KEY_SIZE - _server._svParam.prefixBit;
            assert(bankKeySize > 0);

            /**** create mask value for each specific field*/
            ull maskValue = value;
            ull maskKey   = (((ull)(bankIdx)) << (bankKeySize)) |
                            ((ull)idxInBank);
            ull maskLoad  = isLoad;

            /**** bitwise all component to composed the packet*/
            baseElement |= maskValue;
            baseElement |= (maskKey << valueSize);
            baseElement |= maskLoad << (valueSize + keySize);

            std::cout << "value " << maskValue << " key " << maskKey << " mode " << maskLoad << std::endl;

            return baseElement;
        }

    };



    void startSimpleCacheAcSim(PARAM& params);


}

#endif //KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_SYSTEMTEST_H
