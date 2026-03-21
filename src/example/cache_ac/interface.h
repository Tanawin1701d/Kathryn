//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_PACKETBASE_H
#define KATHRYN_PACKETBASE_H

#include "kathryn.h"
#include "parameter.h"
#include "example/data_struct/field/dynamic_field.h"


namespace kathryn::cache_server{


    struct BankInputInterface: SingleHandShakeBase{
        const KV_PARAM& _param;
        ull bank_id = 0;
        m_reg(is_load, 1);                 Operable* i_is_load = nullptr; ////// pure is_load
        m_reg(key   , _param.KEY_SIZE);   Operable* i_key    = nullptr; ////// pure key

        std::vector<Reg*> values;        std::vector<Operable*> i_value; ////// pure value

        explicit BankInputInterface(KV_PARAM& param, int b_id):
        _param(param),bank_id(b_id){
            for (auto[key_name, sz]: param.valuefield.get_all_key_size()){
                values.push_back(&m_opr_reg(key_name, sz));
            }
        }

        void transfer_pay_load() override{
            is_load <<= *i_is_load;
            key    <<= *i_key;
            /////// check transfer size
            assert(_param.valuefield._valueFieldNames.size() == values.size());
            assert(values.size() == i_value.size());
            for (int i = 0; i < values.size(); i++){
                *values[i] <<= *i_value[i];
            }
        }

        void set_input_param(Operable* l, Operable* k, std::vector<Operable*> vs){
            i_is_load = l; i_key = k; i_value = vs;
        }

        Operable& next_is_load(){
            //////// we need to make sure that decision is correct for next cycle
            return (req_result & *i_is_load) | (~req_result & is_cur_cycle_busy() & is_load);
        }

        ////// for get bank key
        ull get_bank_id(){return bank_id;}

    };

    struct BankOutputInterface: SingleHandShakeBase{
        const KV_PARAM&        _param;
        BankInputInterface&    _inputItf;
        Operable*              result_key   = nullptr;
        std::vector<Operable*> i_values;
        Wire*                  read_en      = nullptr;

        m_wire(out_test, 1);

        explicit BankOutputInterface(
            const KV_PARAM& param,
            BankInputInterface& input_itf
        ): SingleHandShakeBase(false)
        ,_param(param), _inputItf(input_itf){}

        void set_pay_load(Operable* k, std::vector<Operable*> vs, Wire* e){
            result_key = k; i_values = vs; read_en = e;
        }

        void transfer_pay_load() override{
            (*read_en) = 1;
            zif (is_req_success())
                _inputItf.declare_ready_to_rcv();
        }

    };

}

#endif //KATHRYN_PACKETBASE_H