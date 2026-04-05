//
// Created by tanawin on 29/2/2567.
//

#include "modelMode.h"


namespace kathryn{



    ASSIGN_MODE _asMode = AM_MOD;

    ASSIGN_MODE getAssignMode(){
        return _asMode;
    }

    void setAssignMode(ASSIGN_MODE asMode){
        _asMode = asMode;
    }



}