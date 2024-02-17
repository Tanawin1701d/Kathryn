//
// Created by tanawin on 28/11/2566.
//

#include "expression.h"
#include "model/controller/controller.h"

namespace kathryn{

    /**
     * exprMetas
     * */

    expression::expression(LOGIC_OP op,
                           Operable* a,
                           Operable* b,
                           int exp_size):
    LogicComp<expression>({0, exp_size}, TYPE_EXPRESSION,
                          new RtlSimEngine(exp_size, VST_WIRE, false),false),
    _op(op),
    _a(a),
    _b(b)
    {
        com_init();
    }

    expression::expression(int exp_size):
    LogicComp<expression>({0, exp_size}, TYPE_EXPRESSION,
                          new RtlSimEngine(exp_size, VST_WIRE, false), false),
    _op(ASSIGN),
    _a(nullptr),
    _b(nullptr){
        com_init();
    }


    void expression::com_init() {
        ctrl->on_expression_init(this);
    }

    expression& expression::operator=(Operable &b) {
        _a = &b;
        assert(b.getOperableSlice().getSize() == getOperableSlice().getSize());
        return *this;
    }

    expression& expression::operator=(expression& b){
        if (this == &b){
            return *this;
        }
        operator=(*(Operable*)&b);
        return *this;
    }

    SliceAgent<expression>& expression::operator()(int start, int stop) {
        auto ret =  new SliceAgent<expression>(this,
                                               getAbsSubSlice(start, stop, getSlice()));
        return *ret;
    }

    SliceAgent<expression>& expression::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    expression& expression::callBackBlockAssignFromAgent(Operable &b, Slice absSlice) {
        std::cout << "exprMetas should not be assign in slice mode" << std::endl;
        assert(false);
    }

    expression& expression::callBackNonBlockAssignFromAgent(Operable &b, Slice absSlice) {
        std::cout << "exprMetas should not be assign in slice mode" << std::endl;
        assert(false);
    }

    void expression::simStartCurCycle() {


        if (getSimEngine()->isCurValSim()){
            return;
        }
        if (getGlobalId() == 49)
        {
            std::cout << "s49" << std::endl;
        };
        if (getGlobalId() == 29)
        {
            std::cout << "s29" << std::endl;
        };

        getSimEngine()->setCurValSimStatus();

        ValRep  firstValRep(1); /**the size will be change*/
        ValRep  secValRep  (1);
        ValRep& desValRep   = getSimEngine()->getCurVal();
        /**value a*/
        if (_a != nullptr){
            _a->castToRtlSimItf()->simStartCurCycle();
            assert(_a->castToRtlSimItf()->getSimEngine()->isCurValSim());
            firstValRep =  _a->getExactSimCurValue().slice(_a->getOperableSlice());
            if (getGlobalId() == 49)
            {
                std::cout << "get to invert" << _a->getExactSimCurValue().getBiStr() << "    " << &_a->getExactSimCurValue()  << std::endl;
            }
        }
        /**value b*/
        if (_b != nullptr){
            _b->castToRtlSimItf()->simStartCurCycle();
            assert(_b->castToRtlSimItf()->getSimEngine()->isCurValSim());
            secValRep =  _b->getExactSimCurValue().slice(_b->getOperableSlice());
        }


        switch (_op) {

            case BITWISE_AND:
                desValRep = (firstValRep) & (secValRep);
                break;
            case BITWISE_OR:
                desValRep = (firstValRep) | (secValRep);
                break;
            case BITWISE_XOR:
                desValRep = (firstValRep) ^ (secValRep);
                break;
            case BITWISE_INVR:
                desValRep = ~(firstValRep);
                break;
            case BITWISE_SHL:
                desValRep = (firstValRep) << (secValRep);
                break;
            case BITWISE_SHR:
                desValRep = (firstValRep) >> (secValRep);
                break;
            case LOGICAL_AND:
                desValRep = (firstValRep) && (secValRep);
                break;
            case LOGICAL_OR:
                desValRep = (firstValRep) || (secValRep);
                break;
            case LOGICAL_NOT:
                desValRep = !(firstValRep);
                break;
            case RELATION_EQ:
                desValRep = (firstValRep) == (secValRep);
                break;
            case RELATION_NEQ:
                desValRep = (firstValRep) != (secValRep);
                break;
            case RELATION_LE:
                desValRep = (firstValRep) < (secValRep);
                break;
            case RELATION_LEQ:
                desValRep = (firstValRep) <= (secValRep);
                break;
            case RELATION_GE:
                desValRep = (firstValRep) > (secValRep);
                break;
            case RELATION_GEQ:
                desValRep = (firstValRep) >= (secValRep);
                break;
            case ARITH_PLUS:
                desValRep = (firstValRep) + (secValRep);
                break;
            case ARITH_MINUS:
                desValRep = (firstValRep) - (secValRep);
                break;
            case ARITH_MUL:
                desValRep = (firstValRep) * (secValRep);
                break;
            case ARITH_DIV:
                desValRep = (firstValRep) / (secValRep);
                break;
            case ARITH_DIVR:
                desValRep = (firstValRep) % (secValRep);
                break;
            case ASSIGN:
                desValRep = firstValRep;
                break;
            case OP_DUMMY:
            case LOGIC_OP_COUNT:
                break;
        }

        if (getGlobalId() == 29)
        {
            std::cout << "firstCmp" << desValRep.getBiStr() << "       " << &desValRep << std::endl;
        };


        desValRep.fillZeroToValrep(getSlice().getSize());
        assert(getSlice().start == 0);
        assert(desValRep.getLen() == getSlice().getSize());
    }

//    std::vector<std::string> expression::getDebugAssignmentValue() {
//        if (isSingleOpr(_op)){
//            return {lop_to_string(_op) + _a->castToIdent()->getGlobalName()};
//        }else{
//            std::string aName = _a->castToIdent()->getGlobalName();
//            std::string bName = _b->castToIdent()->getGlobalName();
//            return {aName + lop_to_string(_op) + bName};
//        }
//    }
}