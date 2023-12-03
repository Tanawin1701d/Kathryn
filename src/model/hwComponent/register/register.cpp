//
// Created by tanawin on 29/11/2566.
//

#include "register.h"
#include "model/hwComponent/expression/expression.h"
#include "model/controller/controller.h"


namespace kathryn{

    Reg::Reg(int size) : Assignable(),Operable(),
                         Slicable<Reg>(Slice{0, size}),
                         Identifiable(TYPE_REG) {}

    void Reg::com_init() {
        ctrl->on_reg_init(std::shared_ptr<Reg>(this));
    }

    Reg& Reg::operator<<=(Operable &b) {
        /** todo this must call model control system to determine
         * given information and condition of updating value
        /* we will call model building to comunicate with it*/
        //** todo return agent of this type*/
        return *this;
    }

    Reg& Reg::operator=(Operable &b) {
        /** todo first version we not support this operator*/
        return *this;
    }

    /** slicable override*/


    SliceAgent<Reg>& Reg::operator()(int start, int stop) {
        auto ret =  std::make_shared<SliceAgent<Reg>>(
                                            std::shared_ptr<Reg>(this),
                                            Slice{start, stop});
        return *ret;
    }

    SliceAgent<Reg>& Reg::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    Reg& Reg::callBackBlockAssignFromAgent(Operable &b, Slice absSlice) {
        /** todo this must call model control system to determine
         * given information and condition of updating value
        /* we will call model building to comunicate with it*/
        //** todo return agent of this type*/
        return *this;
    }

    Reg &Reg::callBackNonBlockAssignFromAgent(Operable &b, Slice absSlice) {
        return *this;
    }



    /** assign call back*/






}