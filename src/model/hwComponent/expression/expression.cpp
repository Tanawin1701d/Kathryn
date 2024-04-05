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
                           const Operable* a,
                           const Operable* b,
                           int exp_size):
    LogicComp<expression>({0, exp_size},
                          TYPE_EXPRESSION,
                          new expressionLogicSim(this, exp_size,VST_WIRE, false),
                          false),
    _valueAssinged(true),
    _op(op),
    _a(const_cast<Operable *>(a)),
    _b(const_cast<Operable *>(b))
    {
        com_init();
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }

    expression::expression(int exp_size):
    LogicComp<expression>({0, exp_size},
                          TYPE_EXPRESSION,
                          new expressionLogicSim(this, exp_size,VST_WIRE, false),
                          false),
    _valueAssinged(false),
    _op(ASSIGN),
    _a(nullptr),
    _b(nullptr){
        com_init();
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }


    void expression::com_init() {
        ctrl->on_expression_init(this);
    }

    void expression::doNonBlockAsm(Operable &b, Slice desSlice) {
        mfAssert(getAssignMode() == AM_MOD, "expression can use operator = only in MD mode");
        _a = &b;
        assert(b.getOperableSlice().getSize() == getOperableSlice().getSize());
        assert(desSlice.getSize() == getOperableSlice().getSize());
        mfAssert(!_valueAssinged, "multiple expression assign detect");
        _valueAssinged = true;
    }



    SliceAgent<expression>& expression::operator()(int start, int stop) {
        auto ret =  new SliceAgent<expression>(this,
                                               getAbsSubSlice(start, stop, getSlice()));
        return *ret;
    }

    SliceAgent<expression>& expression::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    SliceAgent<expression>& expression::operator()(Slice sl) {
        return operator() (sl.start, sl.stop);
    }

    Operable* expression::doSlice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.castToOperable();
    }

    Operable* expression::checkShortCircuit(){
        if (isInCheckPath){
            return this;
        }
        isInCheckPath = true;

        Operable* result;
        if (_a != nullptr){
            result = _a->checkShortCircuit();
            if (result != nullptr){
                return result;
            }
        }
        if (_b != nullptr){
            result = _b->checkShortCircuit();
            if (result != nullptr){
                return result;
            }
        }

        isInCheckPath = false;
        return nullptr;
    }


    /**
     *
     * expression sim
     * */

    expressionLogicSim::expressionLogicSim(expression* master,
                                           int sz,
                                           VCD_SIG_TYPE sigType,
                                           bool simForNext):
            LogicSimEngine(sz, sigType, simForNext),
            _master(master){}

    void expressionLogicSim::simStartCurCycle() {


        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();

        ValRep  firstValRep(1); /**the size will be change*/
        ValRep  secValRep  (1);
        ValRep& desValRep   = getCurVal();
        /**value a*/
        Operable* a = _master->_a;
        if (a != nullptr){
            a->getSimItf()->simStartCurCycle();
            assert(a->getRtlValItf()->isCurValSim());
            firstValRep =  a->getSlicedCurValue();
        }
        Operable* b = _master->_b;
        /**value b*/
        if (b != nullptr){
            b->getSimItf()->simStartCurCycle();
            assert(b->getRtlValItf()->isCurValSim());
            secValRep = b->getSlicedCurValue();
        }


        switch (_master->getOp()) {

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

        desValRep.fillZeroToValrep(_master->getSlice().getSize());
        assert(_master->getSlice().start == 0);
        assert(desValRep.getLen() == _master->getSlice().getSize());
    }

}