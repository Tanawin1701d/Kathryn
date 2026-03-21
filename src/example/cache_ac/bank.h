//
// Created by tanawin on 11/9/2024.
//

#ifndef KATHRYN_BANK_H
#define KATHRYN_BANK_H

#include "kathryn.h"
#include "interface.h"


    namespace kathryn::cache_server{

        class CacheBankBase: public Module{
        public:
            KV_PARAM&      _kb_param;
            DYNAMIC_FIELD fields; //// valid bit include
            const int     LIMIT_TIME = 60000;
            const int     AMT_WORD   = 1;
            const int     _bankId    = -1;


            int read_count_idx = 0;
            int write_count_idx = 0;

            /////////////// list vector
            std::vector<Operable*>     read_activation;
            std::vector<Operable*>     read_indexers;
            std::vector<Operable*>     write_activation;
            std::vector<Operable*>     write_indexers;
            std::vector<Operable*>     write_values;

            ///////////// <valid bit><value>
            m_mem(pool_data, AMT_WORD, fields.sum_field_size());
            m_reg(timer_cnt, 16);

            /** global memory management*/

            /////// for read
            m_wire(glob_read_indexer, log2Ceil(AMT_WORD));
            m_wire(glob_read_output , fields.sum_field_size());

            /////// for write
            m_wire(glob_write_enable , 1);
            m_wire(glob_write_indexer, log2Ceil(AMT_WORD));
            m_wire(glob_write_value  , fields.sum_field_size());

            m_val(DUMMY_RESET_VALUE, fields.sum_field_size(), 0);

            m_wire(wa, 1);

            CacheBankBase(KV_PARAM& kv_param, const int amount_word, int bank_id):
            _kb_param(kv_param),
            fields   (kv_param.valuefield + DYNAMIC_FIELD({"valid"}, {1})),
            AMT_WORD(amount_word),
            _bankId(bank_id){}

            virtual void                 decode_packet()           = 0; ////// retrieve packet from queue do it your own
            virtual void                 maintenance_bank()        = 0; ////// do  maintenance bank
            virtual BankInputInterface*get_bank_input_interface_ptr()  = 0; ////// get the bank input interface
            virtual BankOutputInterface*get_bank_output_interface_ptr() = 0; ////// get the bank output interface

            void flow() override{
                do_timer();
                do_schedule();
                do_shard_mem();
            }

            void do_timer(){
                zif(get_reset_signal()){
                    timer_cnt <<= 0;
                }
                cwhile(true){
                    zif(timer_cnt == LIMIT_TIME) timer_cnt <<= 0;
                    zelse                       timer_cnt <<= timer_cnt + 1;
                }
            }

            void do_schedule(){
                BankInputInterface* in_itf = get_bank_input_interface_ptr();

                cwhile(true){
                    cif(timer_cnt == LIMIT_TIME){ strack("maintenance" + std::to_string(_bankId))
                        maintenance_bank();
                    }celif(in_itf->is_next_cycle_busy()){ strack("decode" + std::to_string(_bankId));
                        decode_packet();
                    }celse{
                        wa = 1;
                        ////sy_wait(1);
                    }
                }
            }

            void do_shard_mem(){
                //////// read the data

                glob_read_output = pool_data[glob_read_indexer];
                for (int idx = 0; idx < read_activation.size(); idx++){
                    zif (*read_activation[idx]){
                        glob_read_indexer = *read_indexers[idx];
                    }
                }

                //////// write the data
                zif (glob_write_enable){
                    pool_data[glob_write_indexer] <<= glob_write_value;
                }

                glob_write_enable = ~(make_nest_man_read_only(true, write_activation) == 0);
                for (int idx = 0; idx < write_activation.size(); idx++){
                    zif (*write_activation[idx]){
                        glob_write_indexer = *write_indexers[idx];
                        m_val(valid, 1, 1);
                        glob_write_value   = gr(valid, *write_values[idx]);
                    }
                }
            }

            std::vector<Operable*> get_read_value(){
                std::vector<Operable*> result;
                for (std::string value_name: _kb_param.valuefield._valueFieldNames){
                    int idx      = fields.find_idx(value_name);
                    int start_bit = fields.find_start_bit(idx);
                    int stop_bit  = start_bit + fields.get_size(idx);
                    result.push_back(&glob_read_output(start_bit, stop_bit));
                }
                assert(!result.empty());
                return result;
            }

            std::pair<Wire&, std::vector<Operable*>> read_mem(Operable& addr){
                Wire& bit = make_opr_wire("read_en_sig" + std::to_string(read_count_idx++), 1);
                assert(addr.get_operable_slice().get_size() == log2Ceil(AMT_WORD));
                read_activation.push_back(&bit);
                read_indexers.push_back(&addr);
                return {bit, get_read_value()};
            }

            ////// data [a3, a2, a1, a0]
            Operable& composed_data_to_assign(std::vector<Reg*> datas){
                ////// because nest need reverse assumption
                std::reverse(datas.begin(), datas.end());
                std::vector<Operable*> oprs;
                for (Reg* opr: datas){
                    assert(opr != nullptr);
                    oprs.push_back(opr);
                }
                return make_nest_man_read_only(true, oprs);
            }

            void write_mem(Operable& idx, Operable& value){
                Wire& activate_wire = make_opr_wire("write_en_sig" + std::to_string(write_count_idx++), 1) = 1;

                assert(idx.get_operable_slice().get_size() == log2Ceil(AMT_WORD));
                write_activation.push_back(&activate_wire);
                write_indexers  .push_back(&idx);
                write_values    .push_back(&value);
            }

            void reset_mem(Operable& idx){
                write_mem(idx, DUMMY_RESET_VALUE);
            }

            /** this work only for simulation */
            /*** return vector of key and value  ***/
            std::vector<KV_DEBUG> get_active_value_debug(){

                std::vector<KV_DEBUG> result;
                for (int row = 0; row < AMT_WORD; row++){
                        ///// check the valid bit
                        ///////// this is set to < 64
                        ull  read_data = pool_data.at(row).get_val();
                        bool valid    = (read_data >> _kb_param.valuefield.sum_field_size());

                        if (valid){
                            std::string key   = std::to_string(row);
                            std::string value = std::to_string(read_data & (((ull)1 << _kb_param.valuefield.sum_field_size())-1) );
                            result.push_back({key, value});
                        }
                }
                return result;
            }
        };
    }

#endif //KATHRYN_BANK_H