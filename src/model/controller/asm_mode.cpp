//
// Created by tanawin on 24/9/25.
//


#include "asm_mode.h"

#include "model/hw_component/abstract/update_event.h"

namespace kathryn{


    ASM_NODE_PRIORITY_MODE cur_asm_node_priority_mode = ANPM_AUTO;
    int                    cur_asm_node_priority = DEFAULT_UE_PRI_USER;

    ASM_NODE_PRIORITY_MODE GET_ASM_PRI_MODE(){
        return cur_asm_node_priority_mode;
    }
    int GET_ASM_PRI_VAL(){
        return cur_asm_node_priority;
    }
    void SET_ASM_PRI_TO_AUTO(){
        cur_asm_node_priority_mode = ANPM_AUTO;
        cur_asm_node_priority     = DEFAULT_UE_PRI_USER;
    }
    void SET_ASM_PRI_TO_MANUAL(int priority){
        cur_asm_node_priority_mode = ANPM_MANUAL;
        cur_asm_node_priority     = priority;

    }


}
