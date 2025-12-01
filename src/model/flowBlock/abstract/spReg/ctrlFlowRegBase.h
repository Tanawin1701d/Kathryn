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
            ){
            getSimEngine()->setFlowBlockIden(true);
        }


        UpdateEventBase* createUE(Operable* cond,
                                  Operable* state,
                                  Operable* value,
                                  Slice sl,
                                  int priority,
                                  CLOCK_MODE cm);

        /** add depend State register return update event*/
        virtual UpdateEventBase* addDependState(Operable* dependState, Operable* activateCond, CLOCK_MODE cm) = 0;
        /** build register representation when leaving this state */
        virtual void      makeUnSetStateEvent(CLOCK_MODE cm) = 0;
        /***make reset Event */
        virtual void      makeUserRstEvent(Operable* rst, CLOCK_MODE cm) = 0;
        /***generate expression that represent state is finish*/
        virtual Operable* generateEndExpr() = 0;
        /** to check that this register require reset event*/
        virtual bool requireResetEvent(){
            return true;
        }



    };

}

#endif //KATHRYN_CTRLFLOWREGBASE_H
