//
// Created by tanawin on 19/9/2024.
//

#ifndef KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_CACHESLOTWRITER_H
#define KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_CACHESLOTWRITER_H

#include "util/file_writer/slot_writer/slot_writer.h"
#include "example/cache_ac/simple_server/simple_system.h"

namespace kathryn::cache_server{


    constexpr int INGR_SLOT_IDX = 0;
    constexpr int OUTR_SLOT_IDX = 1;
    constexpr int BANK_SLOT_START_IDX = 2;

    class CacheSlotWriter: public SlotWriter{

        SimpleServer& _simpleServer;

    public:

        CacheSlotWriter(
            SimpleServer& simple_server,
            std::string file_name
        ):
        SlotWriter(gen_column(simple_server), 25, file_name),
        _simpleServer(simple_server){

        }

        //////////////////// generate column name
        std::vector<std::string> gen_column(SimpleServer& base_server){

            std::vector<std::string> result = { "ingress", "outgress"};
            for (int bank_idx = 0;
                     bank_idx < base_server.get_ref_banks().size();
                     bank_idx++){
                result.push_back("bankIdx_" + std::to_string(bank_idx));
            }
            return result;
        }

        //////////////////// record base
        void record_slot(){
            record_ingress();
            record_outgress();
            record_bank();
            conclude_each_cycle();

        }
        /////////////////// generate ingress block
        void record_ingress(){
            ///SimpleIngress& ingr = *((SimpleIngress*)_simpleServer._ingress);
            ////////////////////////////// record test

            ////////////////////////////////////////////////////////////////////////////////
            SERVER_PARAM sv_param = _simpleServer._svParam;
            Queue& ingress_queue = _simpleServer._ingress->_qMem;

            add_slot_val(INGR_SLOT_IDX, "req_res_bank0 " + std::to_string((ull)_simpleServer._ingress->_bankInterfaces[0]->req_result));
            add_slot_val(INGR_SLOT_IDX, "req_res_bank1 " + std::to_string((ull)_simpleServer._ingress->_bankInterfaces[1]->req_result));

            add_slot_val(INGR_SLOT_IDX, "req_to_deq " + std::to_string((ull)_simpleServer._ingress->req_to_dequeue));
            add_slot_val(INGR_SLOT_IDX, "deq_intd " + std::to_string((ull)ingress_queue.deq_intend));
            add_slot_val(INGR_SLOT_IDX, "head_pos " + std::to_string((ull)ingress_queue.head_pos));
            add_slot_val(INGR_SLOT_IDX, "last_pos " + std::to_string((ull)ingress_queue.last_pos));
            add_slot_val(INGR_SLOT_IDX, "cur_size " + std::to_string((ull)ingress_queue.cur_size));

            auto queue_debug_value=
                ingress_queue.get_sim_debug(
                        {1, sv_param.kv_param.KEY_SIZE, sv_param.kv_param.valuefield.sum_field_size()}
                );

            for (auto&  queue_ele: queue_debug_value){
                add_slot_val(INGR_SLOT_IDX,
               "m " + queue_ele[2] +
                " k " + queue_ele[1] +
                " v " + queue_ele[0]
                );
            }
        }

        void record_outgress(){
            SimpleOutgress& outgr = *((SimpleOutgress*)_simpleServer._outgress);

            add_slot_val(OUTR_SLOT_IDX, "are_there_fin " + std::to_string((ull)outgr.are_there_fin));
            add_slot_val(OUTR_SLOT_IDX, "cur_bank_idx " + std::to_string((ull)outgr.cur_bank_itr));

            if ( ((ull)outgr.o_valid) == 0 ){
                add_slot_val(OUTR_SLOT_IDX, "nop");
                return;
            }

            add_slot_val(OUTR_SLOT_IDX, "k " + std::to_string((ull)outgr.o_key));

            int idx = 0;
            for (Reg* reg: outgr.o_values){
                add_slot_val(OUTR_SLOT_IDX,
                    "v" + std::to_string(idx) + " " + std::to_string((ull)*reg));
            }

        }

        void record_bank(){

            std::vector<CacheBankBase*> cache_banks = _simpleServer.get_ref_banks();
            for (int bank_idx = 0; bank_idx < cache_banks.size(); bank_idx++){
                assert(cache_banks[bank_idx] != nullptr);
                auto filled_element = cache_banks[bank_idx]->get_active_value_debug();

                BankInputInterface&  input_interface   = ((SimpleBank*)cache_banks[bank_idx])->input_itf;
                BankOutputInterface& output_interface  = ((SimpleBank*)cache_banks[bank_idx])->output_itf;
                SimpleBank&          simple_bank       = *(SimpleBank*)cache_banks[bank_idx];

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "valid " +
                           std::to_string((ull)input_interface.is_busy));
                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "value " +
                           std::to_string((ull)(*input_interface.values[0])));
                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "key " +
                           std::to_string((ull)input_interface.key));
                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "mode " +
                           std::to_string((ull)input_interface.is_load));
                //-----------------------------------------------------------------------------------

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "timer " +
                           std::to_string((ull)(((SimpleBank*)cache_banks[bank_idx])->timer_cnt)));
                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "is_writing " +
                           std::to_string((ull)(((SimpleBank*)cache_banks[bank_idx])->is_writing)));
                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "clean_cnt " +
                           std::to_string((ull)(((SimpleBank*)cache_banks[bank_idx])->clean_cnt)));
                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "out_stuck " +
                           std::to_string((ull)(((SimpleBank*)cache_banks[bank_idx])->output_itf.out_test)));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "wait_cycle " +
                           std::to_string((ull)(((SimpleBank*)cache_banks[bank_idx])->wa)));
                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "is_load " +
                           std::to_string((ull)input_interface.is_load));
                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "ReqResult " +
                           std::to_string((ull)input_interface.req_result));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "is_req_to_send " +
                           std::to_string((ull)input_interface.is_req_to_send()));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "is_valid " +
                           std::to_string((ull)input_interface.is_busy));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "lasat_item_fin " +
                           std::to_string((ull)input_interface.ready_to_get_new));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "------- read Result");

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "result_key" + std::to_string((ull)output_interface.result_key));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "result_value" + std::to_string((ull)(*output_interface.i_values[0])));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                                "ReqResult " + std::to_string((ull)output_interface.req_result));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "is_req_to_send " +
                           std::to_string((ull)output_interface.is_req_to_send()));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "is_valid " +
                           std::to_string((ull)output_interface.is_busy));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "lasat_item_fin " +
                           std::to_string((ull)output_interface.ready_to_get_new));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "glob_read_idx " +
                           std::to_string((ull)simple_bank.glob_read_indexer));

                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "read_activation " +
                           std::to_string((ull)(*simple_bank.read_activation[0])));
                add_slot_val(BANK_SLOT_START_IDX + bank_idx,
                           "read_value " +
                           std::to_string((ull)(*output_interface.i_values[0])));


                for (auto[key, val]: filled_element){
                    add_slot_val(BANK_SLOT_START_IDX + bank_idx, "k " + key + " v " + val);

                }
            }

        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_CACHESLOTWRITER_H
