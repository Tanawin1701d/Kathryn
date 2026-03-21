//
// Created by tanawin on 15/9/2024.
//

#ifndef SIMPLEINGRESS_H
#define SIMPLEINGRESS_H

#include "utility"

#include "example/cache_ac/ingress.h"


namespace kathryn::cache_server{

    class SimpleIngress: public IngressBase{
    public:

        explicit SimpleIngress(
            SERVER_PARAM&                    sv_param,
            std::vector<BankInputInterface*> bank_interfaces):
        IngressBase(sv_param, std::move(bank_interfaces),
        sv_param.kv_param.valuefield +
        DYNAMIC_FIELD({"key", "is_load"}, {sv_param.kv_param.KEY_SIZE, 1})
        ){
            for (auto bank_in_itf : _bankInterfaces){
                bank_in_itf->set_input_param(&_qMem.get_front("is_load"),
                                         &_qMem.get_front("key"),
                                         _qMem.get_front(sv_param.kv_param.valuefield._valueFieldNames)
                                         );
            }
        }

        void flow() override{
            //////// initializing value and indexing
            var queue_avail = ~_qMem.is_empty();
            //////// deal with bank interface
            for (int i = 0; i < _bankInterfaces.size(); i++){
                BankInputInterface* bank_in_itf = _bankInterfaces[i];
                ////// assign req_to_dequeue signal that bank is ready to receive
                req_to_dequeue(i) = bank_in_itf->is_req_success();
                ////// assign send the data
                Slice bank_sl = {_svParam.kv_param.KEY_SIZE - _svParam.prefix_bit,_svParam.kv_param.KEY_SIZE};
                bank_in_itf->send_on(queue_avail & ((*_qMem.get_front("key").do_slice(bank_sl)) == bank_in_itf->bank_id));
            }
            /////// deal with queue
            _qMem.init_logic();
            cwhile(true){
                zif (req_to_dequeue){
                    _qMem.de_queue();
                }
            }
        }
    };

}

#endif //SIMPLEINGRESS_H
