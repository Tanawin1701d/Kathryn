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
                           Operable* a,
                           Slice aSlice,
                           Operable* b,
                           Slice bSlice,
                           int exp_size):
   Assignable<expression>(),
    Operable(),
    Slicable<expression>({0, exp_size}),
    Identifiable(TYPE_EXPRESSION),
    _op(op),
    _a(a),
    _aSlice(aSlice),
    _b(b),
    _bSlice(bSlice)
    {
        com_init();
    }

    expression::expression():
    Assignable<expression>(),
    Operable(),
    Slicable<expression>(Slice()),
    Identifiable(TYPE_EXPRESSION),
    _op(ASSIGN),
    _a(nullptr),
    _aSlice(Slice()),
    _b(nullptr),
    _bSlice(Slice()){
        com_init();
    }


    void expression::com_init() {
        ctrl->on_expression_init(this);
    }

    expression& expression::operator=(Operable &b) {
        _a = &b;
        Slice proxySlice({0, b.getOperableSlice().getSize()});
        _aSlice = proxySlice;
        setSlice(proxySlice);
        return *this;
    }

    SliceAgent<expression>& expression::operator()(int start, int stop) {
        auto ret =  new SliceAgent<expression>(this,
                                               getNextSlice(start, stop, getSlice()));
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