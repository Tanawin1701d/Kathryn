//
// Created by tanawin on 3/1/2567.
//

#ifndef KATHRYN_CTRLFLOWREGBASE_H
#define KATHRYN_CTRLFLOWREGBASE_H

#include "model/hwComponent/register/register.h"


namespace kathryn{

    enum SP_REG_TYPE{
        SP_STATE_REG = 0,
        SP_SYNC_REG = 1,
        SP_COND_WAIT_REG = 2,
        SP_CYCLE_WAIT_REG = 3,
        SP_CNT_REG = 4

    };

    std::string sp_reg_type_to_str(SP_REG_TYPE spRegType);

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

        /** add depend State register return update event*/
        virtual UpdateEvent* addDependState(Operable* dependState, Operable* activateCond) = 0;
        /** build register representation when leaving this state */
        virtual void      makeUnSetStateEvent() = 0;
        /*** make UNset event that expression make interupt to start sequence*/
        virtual void      makeResetInteruptEvent(Operable* stopTrigger) = 0;
        /*** generate expression that represent state is finish*/
        virtual Operable* generateEndExpr() = 0;

        /** to check that this register require reset event*/
        virtual bool requireResetEvent(){
            return true;
        }



    };

}

#endif //KATHRYN_CTRLFLOWREGBASE_H
