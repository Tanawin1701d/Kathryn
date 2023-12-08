//
// Created by tanawin on 29/11/2566.
//

#include "register.h"
#include "model/hwComponent/expression/expression.h"
#include "model/controller/controller.h"


namespace kathryn{

    /** constructor need to init communication with controller*/
    Reg::Reg(int size, bool initCom) : Assignable(),Operable(),
                         Slicable<Reg>(Slice{0, size}),
                         Identifiable(TYPE_REG) {
        if (initCom) {
            com_init();
        }
    }

    void Reg::com_init() {
        ctrl->on_reg_init(this);
    }

    Reg& Reg::operator<<=(Operable &b) {
        /** todo this must call model control system to determine
         * given information and condition of updating value
        /* we will call model building to comunicate with it*/
        //** todo return agent of this type*/
        ctrl->on_reg_update(generateAssignMeta(b, getSlice()));
        return *this;
    }

    Reg& Reg::operator=(Operable &b) {
        /** todo first version we not support this operator*/
        assert(true);
        return *this;
    }

    /** slicable override*/


    SliceAgent<Reg>& Reg::operator()(int start, int stop) {
        auto ret =  new SliceAgent<Reg>(this,
                                        Slice{start, stop});
        return *ret;
    }

    SliceAgent<Reg>& Reg::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    Reg& Reg::callBackBlockAssignFromAgent(Operable &b, Slice absSlice) {
        ctrl->on_reg_update(generateAssignMeta(b, absSlice));
        return *this;
    }

    Reg &Reg::callBackNonBlockAssignFromAgent(Operable &b, Slice absSlice) {
        assert(true);
        return *this;
    }



    /** assign call back*/






}