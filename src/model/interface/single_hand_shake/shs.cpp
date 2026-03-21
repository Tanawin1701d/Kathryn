//
// Created by tanawin on 3/10/2024.
//

#include "model/flow_block/cond/zif.h"
#include "model/flow_block/cond/zelif.h"
#include "model/flow_block/loop/do_while_base.h"
#include "shs.h"

namespace kathryn{

    void SingleHandShakeBase::build_logic_base(){
        if (_autoAccept){
            req_result = (~is_busy | ready_to_get_new) & req_to_send;
        }else{
            req_result = ready_to_get_new & req_to_send;
        }
        zif(req_result){
            is_busy <<= 1;
            transfer_pay_load();
        }zelif(ready_to_get_new){
            is_busy <<= 0;
        }
        build_user_logic();
    }

    void SingleHandShakeBase::send_and_wait_untill_success(){

        cdowhile(~is_req_success()){
            send();
        }

    }

}