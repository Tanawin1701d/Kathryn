//
// Created by tanawin on 24/9/25.
//


#include "asmMode.h"
#include "model/hwComponent/abstract/updateEvent.h"

namespace kathryn{


    ASM_NODE_PRIORITY_MODE curAsmNodePriorityMode = ANPM_AUTO;
    int                    curAsmNodePriority = DEFAULT_UE_PRI_USER;

    ASM_NODE_PRIORITY_MODE GET_ASM_PRI_MODE(){
        return curAsmNodePriorityMode;
    }
    int GET_ASM_PRI_VAL(){
        return curAsmNodePriority;
    }
    void SET_ASM_PRI_TO_AUTO(){
        curAsmNodePriorityMode = ANPM_AUTO;
        curAsmNodePriority     = DEFAULT_UE_PRI_USER;
    }
    void SET_ASM_PRI_TO_MANUAL(int priority){
        curAsmNodePriorityMode = ANPM_MANUAL;
        curAsmNodePriority     = priority;

    }


}
