//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_SYSTEM_H
#define KATHRYN_SYSTEM_H

#include "bank.h"
#include "ingress.h"
#include "outgress.h"
#include "parameter.h"
#include "vector"

namespace kathryn::cache_server{

        class ServerBase: public Module{
        public:
            SERVER_PARAM&                      _svParam;
            IngressBase*                      _ingress  = nullptr;
            OutgressBase*                     _outgress = nullptr;
            std::vector<CacheBankBase*>       _banks;
            std::vector<BankInputInterface*>  _bankInputItfs;  //// bank input  interfaces
            std::vector<BankOutputInterface*> _bankOutputItfs; //// bank output interfaces
            ///////// constructor
            explicit ServerBase(SERVER_PARAM& sv_param):_svParam(sv_param){
                _svParam.kv_param.valuefield.reverse();
            }
            ///////// start build the element
            void init_server(){
                int amt_bank = 1 << _svParam.prefix_bit;
                for (int bank_id = 0; bank_id < amt_bank; bank_id++) {
                    _banks         .push_back(gen_bank(bank_id));
                    _bankInputItfs .push_back(_banks[bank_id]->get_bank_input_interface_ptr());
                    _bankOutputItfs.push_back(_banks[bank_id]->get_bank_output_interface_ptr());
                }
                _ingress  =  gen_ingress();
                _outgress =  gen_outgress();
            }
            ///////// gen element that used to init server
            virtual CacheBankBase* gen_bank    (int idx) = 0;
            virtual IngressBase*   gen_ingress ()        = 0;
            virtual OutgressBase*  gen_outgress()        = 0;

            ///////// getter
            IngressBase& get_ingress() const{
                assert(_ingress != nullptr);
                return *_ingress;
            }
            OutgressBase& get_outgress() const{
                assert(_outgress != nullptr);
                return *_outgress;
            }
            std::vector<CacheBankBase*>& get_ref_banks(){return _banks;}
        };

    }


#endif //KATHRYN_SYSTEM_H
