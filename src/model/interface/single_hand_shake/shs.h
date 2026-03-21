//
// Created by tanawin on 3/10/2024.
//

#ifndef src_model_interface_singleHandShake_SHS_H
#define src_model_interface_singleHandShake_SHS_H



#include "model/hw_component/box/box.h"
#include "model/hw_component/abstract/make_component.h"
#include "model/hw_component/register/register.h"
#include "model/hw_component/wire/wire.h"
#include "model/hw_component/expression/expression.h"

#include "model/interface/base/interface.h"

namespace kathryn{


    class SingleHandShakeBase: public ModelInterface{
    public:
        bool _autoAccept = true; ////// accept immediately when not busy
        ///// inside datastruct
        m_reg(is_busy, 1);
        ///// recv react
        m_wire(ready_to_get_new, 1);
        m_wire(req_result    , 1);
        //////transceiver
        m_wire(req_to_send    , 1);

        SingleHandShakeBase(bool auto_acc = true): _autoAccept(auto_acc){}

        void build_logic_base() override;
        void send(){ req_to_send = 1;}
        void send_on(Operable& opr1){
            assert(opr1.get_operable_slice().get_size() == 1);
            req_to_send = opr1;
        }
        void send_and_wait_untill_success();



        void declare_ready_to_rcv(){ ready_to_get_new = 1; }

        //////////// get method
        Operable& is_cur_cycle_busy(){ return is_busy;}
        Operable& is_next_cycle_busy(){return (is_busy & (~ready_to_get_new)) | req_result; }
        Operable& is_req_success  (){ return req_result;}
        Operable& is_req_to_send   (){ return req_to_send;}


    };







}

#endif //src_model_interface_singleHandShake_SHS_H
