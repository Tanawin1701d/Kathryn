//
// Created by tanawin on 15/9/2024.
//

#ifndef SIMPLEOUTGRESS_H
#define SIMPLEOUTGRESS_H

#include "utility"

#include "kathryn.h"
#include "example/cache_ac/outgress.h"



namespace kathryn::cache_server{

    class SimpleOutgress: public OutgressBase{
    public:

        const int PREFIX_BIT = -1;

        m_reg(cur_bank_itr, PREFIX_BIT);
        m_reg(o_key      , _svParam.kv_param.KEY_SIZE);
        std::vector<Reg*> o_values;
        m_reg(cur_res_bank, PREFIX_BIT);
        m_reg(o_valid, 1);
        m_wire(are_there_fin, _outputInterfaces.size());

        explicit SimpleOutgress(SERVER_PARAM& sv_param,
                                std::vector<BankOutputInterface*> output_interfaces):
        OutgressBase(sv_param, std::move(output_interfaces)),
        PREFIX_BIT(sv_param.prefix_bit){
            assert(PREFIX_BIT > 0);
            for (auto [key, size] :_svParam.kv_param.valuefield.get_all_key_size()){
                o_values.push_back(&make_opr_reg(key, size));
            }
        }

        void flow() override{

            zif(get_reset_signal()){
                cur_bank_itr <<= 0;
                o_valid     <<= 0;
            }

            for (int idx = 0; idx < _outputInterfaces.size(); idx++){
                zif( (cur_bank_itr == idx) & _outputInterfaces[idx]->is_req_to_send()
                ){  ////// tell that we are finish
                    BankOutputInterface& output_itf = *_outputInterfaces[idx];

                    o_key             <<= *_outputInterfaces[idx]->result_key;
                    cur_res_bank       <<= cur_bank_itr;
                    are_there_fin(idx) = 1;
                    output_itf.declare_ready_to_rcv();
                    int field_idx = 0;
                    for (Operable* value_opr: output_itf.i_values){
                        *o_values[field_idx] <<= *value_opr;
                        field_idx++;
                    }
                }
            }
            cur_bank_itr <<= cur_bank_itr + 1;

            zif (are_there_fin) o_valid <<= 1;
            zelse o_valid <<= 0;


        }
    };

}

#endif //SIMPLEOUTGRESS_H
