//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_EXPRESSION_H
#define KATHRYN_EXPRESSION_H


#include <memory>
#include <string>
#include <iostream>
#include <utility>
#include "model/hwComponent/abstract/slicable.h"
#include "model/hwComponent/abstract/operation.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/identifiable.h"
#include "model/controller/conInterf/controllerItf.h"


/**
 * expression is the class that represent the value from hardware
 * component such as register and wire
 *
 * */

namespace kathryn {




    class expression : public Assignable<expression>, public Operable,
                       public Slicable<expression>,
                       public AssignCallbackFromAgent<expression>,
                       public Identifiable,
                       public HwCompControllerItf{
    private:

        /** meta data that contain bi operation*/
        LOGIC_OP _op;
        Operable* _a;
        Slice _aSlice{};
        Operable* _b;
        Slice _bSlice{};

    protected:
        void com_init() override;


    public:
        /** constructor auto get id of the system*/
        explicit expression(LOGIC_OP,
                            Operable* a,
                            Slice aSlice,
                            Operable* b,
                            Slice bSlice,
                            int exp_size
                            );
        explicit expression();
        /** override assignable*/
        expression& operator <<= (Operable& b) override {std::cout << "we not support <<= operator in expression";};
        expression& operator =   (Operable& b) override;
        /**override operable*/
        Operable& getExactOperable() override { return *(Operable*)(this); };
        Slice getOperableSlice() override { return getSlice(); }
        /** override slicable*/
        SliceAgent<expression>& operator() (int start, int stop) override;
        SliceAgent<expression>& operator() (int idx) override;
        /** call back assignable from client agent*/
        [[maybe_unused]]
        expression& callBackBlockAssignFromAgent(Operable& b, Slice absSlice) override;
        expression& callBackNonBlockAssignFromAgent(Operable& b, Slice absSlice) override;

    };


}

#endif //KATHRYN_EXPRESSION_H
