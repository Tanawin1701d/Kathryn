//
// Created by tanawin on 3/1/2567.
//

#ifndef KATHRYN_CTRLFLOWREGBASE_H
#define KATHRYN_CTRLFLOWREGBASE_H

#include "model/hwComponent/register/register.h"
#include "model/flowBlock/abstract/nodes/nodeCon.h"


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
        virtual void makeSetEvent    (std::vector<NODE_META>& setMeta) = 0;
        virtual void makeUnsetEvent  () = 0;
        virtual void makeResetIntrSeq(std::vector<NODE_META>& setMeta) = 0;
        virtual void makeStartIntrSeq(std::vector<NODE_META>& setMeta) = 0;
        /**add depend State*/
        virtual void addDependState(Operable* state, Operable* condition) = 0;


        /*** generate expression that represent state is finish*/
        virtual Operable* generateState() = 0;
        /** to check that this register require reset event*/
        virtual bool requireResetEvent(){
            return true;
        }



    };

}

#endif //KATHRYN_CTRLFLOWREGBASE_H
