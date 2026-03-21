//
// Created by tanawin on 19/9/2024.
//

#ifndef KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_SYSTEMTEST_H
#define KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_SYSTEMTEST_H

#include "kathryn.h"
#include "example/cache_ac/simple_server/simple_system.h"
#include "cache_slot_writer.h"

namespace kathryn::cache_server{


    constexpr char VCD_FILE_PARAM        [] = "vcd_file" ;
    constexpr char PROF_FILE_PARAM       [] = "prof_file";
    constexpr char CACHE_SLOT_FILE_PARAM [] = "slot_file";

    class CacheSimItf: public SimInterface{

        SimpleServer&   _server;
        CacheSlotWriter _cacheSlotWriter;


    public:

        CacheSimItf(PARAM& params, SimpleServer& server):
        SimInterface(600,
                     params[VCD_FILE_PARAM],
                     params[PROF_FILE_PARAM],
                     "cache_model",
                     get_spbm(params)
                     ),
        _server(server),
        _cacheSlotWriter(_server, params[CACHE_SLOT_FILE_PARAM])
        {}

        //////void describe_def() override{}

        void describe() override{
            /////////// we must prevent this because the queue meta data will be reset
            inc_cycle(1);
            //////////////   provide data to memory

            sim {
                //// get queue
                Queue &queue = _server.get_ingress()._qMem;
                //// create meta data
                int BANK_AMT = 1 << _server._svParam.prefix_bit;
                int AMT_PER_BANK = 1 << (_server._svParam.kv_param.KEY_SIZE - _server._svParam.prefix_bit);
                ///// push data to the queue
                for (int idx = 0; idx < queue.WORD_AMT; idx++) {
                    queue.push_data_sim(
                            gen_income_packet(
                                    idx % BANK_AMT,
                                    idx % AMT_PER_BANK,
                                    idx,
                                    false) ///// for now we set all element to write
                    );
                }
            };

            inc_cycle(100);

            sim {

                Queue& queue = _server.get_ingress()._qMem;
                int BANK_AMT = 1 << _server._svParam.prefix_bit;
                int AMT_PER_BANK = 1 << (_server._svParam.kv_param.KEY_SIZE - _server._svParam.prefix_bit);

                for (int idx = queue.WORD_AMT-1; idx >= 0; idx--){
                    queue.push_data_sim(
                            gen_income_packet(
                                    idx % BANK_AMT,
                                    idx % AMT_PER_BANK,
                                    idx,
                                    true) ///// for now we set all element to write
                    );
                }

            };


        }

        void describe_con() override{
            //////////////   record slot
            for (int cycle = 1; cycle < 500; cycle++){
                con_end_cycle();
                _cacheSlotWriter.record_slot();
                con_next_cycle(1);
            }
        }



        ull gen_income_packet(int bank_idx, int idx_in_bank, int value, bool is_load){
            ull base_element = 0;

            auto& sp = _server._svParam;

            int sum_value_sz  = sp.kv_param.valuefield.sum_field_size();
            int key_size     = sp.kv_param.KEY_SIZE;
            int bank_key_size = sp.kv_param.KEY_SIZE - _server._svParam.prefix_bit;
            assert(bank_key_size > 0);

            /**** create mask value for each specific field*/
            ull mask_value = 0;
            ull mask_key   = (((ull)(bank_idx)) << (bank_key_size)) |
                            ((ull)idx_in_bank);
            ull mask_load  = is_load;

            /**** bitwise all component to composed the packet*/
            int amt_field = sp.kv_param.valuefield.amt_field();
            for (int fid = amt_field-1; fid >= 0; fid--){
                mask_value  = mask_value << sp.kv_param.valuefield.get_size(fid);
                mask_value  = mask_value | (value  + 2*fid);
            }
            base_element |= mask_value;
            base_element |= (mask_key << sum_value_sz);
            base_element |= mask_load << (sum_value_sz + key_size);

            std::cout << "value " << mask_value << " key " << mask_key << " mode " << mask_load << std::endl;

            return base_element;
        }

    };



    void start_simple_cache_ac_sim(PARAM& params);


}

#endif //KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_SYSTEMTEST_H
