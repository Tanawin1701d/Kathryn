//
// Created by tanawin on 24/9/25.
//

#ifndef KATHRYN_SRC_MODEL_CONTROLLER_ASMNODE_H
#define KATHRYN_SRC_MODEL_CONTROLLER_ASMNODE_H


namespace kathryn{

    ///// this is global system to specify the priority when do the assignment
    enum ASM_NODE_PRIORITY_MODE{
        ANPM_AUTO, ///// default for user is 10
        ANPM_MANUAL
    };

    ASM_NODE_PRIORITY_MODE GET_ASM_PRI_MODE();
    int                    GET_ASM_PRI_VAL();
    void                   SET_ASM_PRI_TO_AUTO();
    void                   SET_ASM_PRI_TO_MANUAL(int priority);


}

#endif //KATHRYN_SRC_MODEL_CONTROLLER_ASMNODE_H
