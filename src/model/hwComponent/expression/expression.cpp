//
// Created by tanawin on 28/11/2566.
//

#include "expression.h"
#include "model/controller/controller.h"

#include <utility>


namespace kathryn{

    /**
     * expression
     * */

    expression::expression(LOGIC_OP op,
                           std::shared_ptr<Operable> a,
                           Slice aSlice,
                           std::shared_ptr<Operable> b,
                           Slice bSlice,
                           int exp_size):
   Assignable<expression>(),
    Operable(),
    Slicable<expression>({0, exp_size}),
    Identifiable(TYPE_EXPRESSION),
    _op(op),
    _a(std::move(a)),
    _aSlice(aSlice),
    _b(std::move(b)),
    _bSlice(bSlice)
    {
        com_init();
    }

    void expression::com_init() {
        ctrl->on_expression_init(expressionPtr (this));
    }

    expression& expression::operator=(Operable &b) {
        return *this;
    }

    SliceAgent<expression>& expression::operator()(int start, int stop) {
        auto ret =  std::make_shared<SliceAgent<expression>>(
                                std::shared_ptr<expression>(this),
                                getSlice()
                );
        return *ret;
    }

    SliceAgent<expression>& expression::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    expression& expression::callBackBlockAssignFromAgent(Operable &b, Slice absSlice) {
        std::cout << "expression should not be assign in slice mode" << std::endl;
        assert(true);
        return *this;
    }

    expression& expression::callBackNonBlockAssignFromAgent(Operable &b, Slice absSlice) {
        std::cout << "expression should not be assign in slice mode" << std::endl;
        assert(true);
        return *this;
    }

}