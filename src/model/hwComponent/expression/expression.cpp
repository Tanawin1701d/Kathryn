//
// Created by tanawin on 28/11/2566.
//

#include "expression.h"
#include "model/controller/controller.h"

#include <utility>


namespace kathryn{

    /**
     * exprMetas
     * */

    expression::expression(LOGIC_OP op,
                           Operable* a,
                           Slice aSlice,
                           Operable* b,
                           Slice bSlice,
                           int exp_size):
    LogicComp<expression>({0, exp_size}, TYPE_EXPRESSION,
                          new RtlSimEngine(exp_size, VST_WIRE),false),
    _op(op),
    _a(a),
    _aSlice(aSlice),
    _b(b),
    _bSlice(bSlice)
    {
        com_init();
    }

    expression::expression():
    LogicComp<expression>(Slice(), TYPE_EXPRESSION, nullptr, false),
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
        int newSize = b.getOperableSlice().getSize();
        Slice proxySlice({0, newSize});
        _aSlice = proxySlice;
        setSlice(proxySlice);
        setSimEngine(new RtlSimEngine(newSize, VST_WIRE));
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

        if (isCurCycleSimulated()){
            return;
        }
        setSimStatus();
        ValRep* firstValRep = nullptr;
        ValRep* secValRep   = nullptr;
        ValRep& desValRep   = getSimEngine()->getCurVal();
        /**value a*/
        if (_a != nullptr){
            _a->castToRtlSimItf()->simStartCurCycle();
            firstValRep =  &(_a->castToRtlSimItf()->getSimEngine()->getCurVal());
        }
        /**value b*/
        if (_b != nullptr){
            _b->castToRtlSimItf()->simStartCurCycle();
            secValRep = &(_b->castToRtlSimItf()->getSimEngine()->getCurVal());
        }

        switch (_op) {

            case BITWISE_AND:
                desValRep = (*firstValRep) & (*secValRep);
                break;
            case BITWISE_OR:
                desValRep = (*firstValRep) | (*secValRep);
                break;
            case BITWISE_XOR:
                desValRep = (*firstValRep) ^ (*secValRep);
                break;
            case BITWISE_INVR:
                desValRep = ~(*firstValRep);
                break;
            case BITWISE_SHL:
                desValRep = (*firstValRep) << (*secValRep);
                break;
            case BITWISE_SHR:
                desValRep = (*firstValRep) >> (*secValRep);
                break;
            case LOGICAL_AND:
                desValRep = (*firstValRep) && (*secValRep);
                break;
            case LOGICAL_OR:
                desValRep = (*firstValRep) || (*secValRep);
                break;
            case LOGICAL_NOT:
                desValRep = !(*firstValRep);
                break;
            case RELATION_EQ:
                desValRep = (*firstValRep) == (*secValRep);
                break;
            case RELATION_NEQ:
                desValRep = (*firstValRep) != (*secValRep);
                break;
            case RELATION_LE:
                desValRep = (*firstValRep) < (*secValRep);
                break;
            case RELATION_LEQ:
                desValRep = (*firstValRep) <= (*secValRep);
                break;
            case RELATION_GE:
                desValRep = (*firstValRep) > (*secValRep);
                break;
            case RELATION_GEQ:
                desValRep = (*firstValRep) >= (*secValRep);
                break;
            case ARITH_PLUS:
                desValRep = (*firstValRep) + (*secValRep);
                break;
            case ARITH_MINUS:
                desValRep = (*firstValRep) - (*secValRep);
                break;
            case ARITH_MUL:
                desValRep = (*firstValRep) * (*secValRep);
                break;
            case ARITH_DIV:
                desValRep = (*firstValRep) / (*secValRep);
                break;
            case ARITH_DIVR:
                desValRep = (*firstValRep) % (*secValRep);
                break;
            case ASSIGN:
                desValRep = *firstValRep;
                break;
            case OP_DUMMY:
            case LOGIC_OP_COUNT:
                break;
        }

    }

    void expression::simExitCurCycle() {
        resetSimStatus();
        getSimEngine()->iterate();
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