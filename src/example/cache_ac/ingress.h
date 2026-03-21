//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_INGRESS_H
#define KATHRYN_INGRESS_H

#include "utility"

#include "kathryn.h"
#include "interface.h"
#include "example/data_struct/queue/queue.h"

namespace kathryn::cache_server{

    class IngressBase: public Module{
    public:
        const SERVER_PARAM& _svParam;
        ////////////////////////////////////////////
        Queue _qMem;// TYPICALLY  /// | is_load<1> | key | value
        ////// to do make input egress
        std::vector<BankInputInterface*> _bankInterfaces;
        m_wire(req_to_dequeue, _bankInterfaces.size());

        IngressBase(
            SERVER_PARAM&                     sv_param,
            std::vector<BankInputInterface*> bank_interfaces,
            DYNAMIC_FIELD                    dy_field):
        _svParam       (sv_param),
        _qMem          (_svParam.ingr_queue_size,dy_field),
        _bankInterfaces(bank_interfaces){}

        virtual int  get_available_bank(){return (int)_bankInterfaces.size();}
    };

}

#endif //KATHRYN_INGRESS_H