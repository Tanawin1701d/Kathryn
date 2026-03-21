//
// Created by tanawin on 29/2/2567.
//

#include "model_mode.h"


namespace kathryn{



    ASSIGN_MODE _asMode = AM_MOD;

    ASSIGN_MODE get_assign_mode(){
        return _asMode;
    }

    void set_assign_mode(ASSIGN_MODE as_mode){
        _asMode = as_mode;
    }



}