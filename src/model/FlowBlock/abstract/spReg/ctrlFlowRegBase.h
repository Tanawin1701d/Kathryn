//
// Created by tanawin on 3/1/2567.
//

#ifndef KATHRYN_CTRLFLOWREGBASE_H
#define KATHRYN_CTRLFLOWREGBASE_H

#include "model/hwComponent/register/register.h"


namespace kathryn{

    class CtrlFlowRegBase : public Reg{

    public:
        /** constructor to initialize register*/
        explicit CtrlFlowRegBase(int regSize,
                                 bool initCom,
                                 HW_COMPONENT_TYPE hw_type,
                                 bool requireAlloc):
            Reg(
                    regSize,
                    initCom,
                    hw_type,
                    requireAlloc
            ){}

        /** build register representation when leaving this state */
        virtual void      makeResetEvent() = 0;
        /***generate expression that represent state is finish*/
        virtual Operable* generateEndExpr() = 0;



    };

}

#endif //KATHRYN_CTRLFLOWREGBASE_H
