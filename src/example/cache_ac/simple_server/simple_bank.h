//
// Created by tanawin on 14/9/2024.
//

#ifndef SIMPLEBANK_H
#define SIMPLEBANK_H


#include "../bank.h"

namespace kathryn::cache_server{

    class SimpleBank: public CacheBankBase{
    public:
        const int           _suffixBit = -1; ///// size of suffix bit
        BankInputInterface  input_itf;
        BankOutputInterface output_itf;
        Operable&           input_in_bank_key; //// only key that used to indicate bank
        m_reg(clean_cnt  , _suffixBit);
        m_wire(is_writing, 1         );
        //////////////////////////////////////////////////////////
        ///   | valid_bit | data_bit |
        //////////////////////////////////////////////////////////

        explicit SimpleBank(KV_PARAM& kv_param, int suffix_bit, int bank_id):
        CacheBankBase (kv_param, 1 << suffix_bit, bank_id),
        _suffixBit    (suffix_bit),
        input_itf      (kv_param, _bankId),
        output_itf     (kv_param, input_itf),
        input_in_bank_key(input_itf.key(0, _suffixBit))
        {   ////// build size of
            assert(suffix_bit > 0);
            assert(bank_id < (1 << suffix_bit));
            ////// build Read Structure

        }

        //// Both decode_packet and maintenance bank are created in flow stage
        void decode_packet() override{

            cif (input_itf.next_is_load()){ strack("loading" + std::to_string(_bankId)) ////// is load /////try until outgress is recv
                auto [en_value, read_values] = read_mem(input_in_bank_key);
                output_itf.set_pay_load(&input_itf.key, read_values, &en_value);
                output_itf.send_and_wait_untill_success();
            }celse{ strack("writing" + std::to_string(_bankId)) //// is write
                ///// write to memory now
                if (_kb_param.replace_pol == OVER_WRITE){
                    write_mem(input_in_bank_key,
                             composed_data_to_assign(input_itf.values));
                }else{ ////// avoid conflict policy
//                    zif(get_valid_bit(input_in_bank_key)){
//                        write_mem(input_in_bank_key, input_itf.value);
                    write_mem(input_in_bank_key, composed_data_to_assign(input_itf.values));
                }

                is_writing = 1;
                input_itf.declare_ready_to_rcv();
            }

        }

        void maintenance_bank() override{
            seq{
                clean_cnt <<= 0;
                cdowhile(clean_cnt != ( (1 << _suffixBit) - 1)){
                    par{
                        clean_cnt <<= clean_cnt + 1;
                        reset_mem(clean_cnt);
                    }
                }
            }
        }

        BankInputInterface*  get_bank_input_interface_ptr () override{return &input_itf;}
        BankOutputInterface*get_bank_output_interface_ptr() override{return &output_itf;}

    };

}

#endif //SIMPLEBANK_H