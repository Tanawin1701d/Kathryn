//
// Created by tanawin on 26/6/2024.
//

#include "kathryn.h"
#include "gen_ele.h"

#include "test/auto_gen/gen_mng.h"

namespace kathryn{


    GenEle::GenEle(int id): _id(id){
        add_gen_ele(this);
    }


    void GenEle::start_gen(PARAM& params){

        start(params);
        start_model_kathryn();
        GenController* gen_ctrl = get_gen_controller_ptr();
        assert(gen_ctrl != nullptr);
        gen_ctrl->init_env(params);
        gen_ctrl->start();
        reset_kathryn();


    }

}
