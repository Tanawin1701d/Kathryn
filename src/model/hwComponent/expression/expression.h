//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_EXPRESSION_H
#define KATHRYN_EXPRESSION_H


#include <memory>
#include <string>
#include <iostream>
#include <utility>

#include "model/controller/conInterf/controllerItf.h"
#include "model/hwComponent/abstract/logicComp.h"


/**
 * exprMetas is the class that represent the value from hardware
 * component such as register and wire
 *
 * */

namespace kathryn {




    class expression : public LogicComp<expression>{
    private:

        /** metas data that contain bi operation*/
        LOGIC_OP _op;
        Operable* _a;
        Slice _aSlice{};
        Operable* _b;
        Slice _bSlice{};

    protected:
        void com_init() override;
        void com_final() override {};


    public:
        /** constructor auto get id of the system*/
        explicit expression(LOGIC_OP op,
                            Operable* a,
                            Slice aSlice,
                            Operable* b,
                            Slice bSlice,
                            int exp_size
                            );
        explicit expression();
        /** override assignable*/
        expression& operator <<= (Operable& b) override {std::cout << "we not support <<= operator in exprMetas"; assert(false);}
        expression& operator =   (Operable& b) override;
        /**override operable*/
        [[nodiscard]]
        Operable& getExactOperable() const override { return *(Operable*)(this); };
        [[nodiscard]]
        Slice getOperableSlice() const override  { return getSlice(); }

        /** override slicable*/
        SliceAgent<expression>& operator() (int start, int stop) override;
        SliceAgent<expression>& operator() (int idx) override;
        /** call back assignable from client agent*/
        [[maybe_unused]]
        expression& callBackBlockAssignFromAgent(Operable& b, Slice absSlice) override;
        expression& callBackNonBlockAssignFromAgent(Operable& b, Slice absSlice) override;
        /** override debugg message*/
        //std::vector<std::string> getDebugAssignmentValue() override;

        /** get set method */
        LOGIC_OP getOp() const {return _op;};
        Operable* getOperandA() const {return _a;}
        Operable* getOperandB() const {return _b;}

        /** override simulator interface*/
        void simStartCurCycle() override;
        void simExitCurCycle() override;

    };


}

#endif //KATHRYN_EXPRESSION_H
