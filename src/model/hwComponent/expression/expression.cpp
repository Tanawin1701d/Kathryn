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
                          new expressionSimEngine(this, VST_WIRE),
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
                          new expressionSimEngine(this, VST_WIRE),
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

    expressionSimEngine::expressionSimEngine(expression* master,
                                           VCD_SIG_TYPE sigType):
            LogicSimEngine(master, master, sigType, false, 0),
            _master(master){
        assert(master != nullptr);
    }

    void expressionSimEngine::proxyBuildInit(){

        /////// add dependency
        assert(_master->_a != nullptr);
        dep.push_back(_master->_a->getLogicSimEngineFromOpr());
        if (_master->_b != nullptr){
            dep.push_back(_master->_b->getLogicSimEngineFromOpr());
        }

    }

    std::string expressionSimEngine::createGlobalVariable(){
        //////////////// for expression
        return "";
    }

    std::string expressionSimEngine::createLocalVariable(){
        /////////// we bring the variable to local variable
        return LogicSimEngine::createGlobalVariable();
    }




    std::string expressionSimEngine::createOp(){

        std::string retStr = "      { /////" + _ident->getGlobalName() + "\n";


        assert(_asb   ->getAssignSlice().getSize());
        assert(_master->_a->getOperableSlice().checkValidSlice());

        retStr += "     ";
        retStr += getVarName();
        retStr += " = ";

        std::string _aSliced = getSliceStringFromOpr(_master->_a);

        std::string _bSliced;

        if (_master->_b != nullptr){
            _bSliced = getSliceStringFromOpr(_master->_b, _master->_a->getOperableSlice().getSize());
        }

        int desSize = _master->getSlice().getSize();


        ////// all operand expect to be equal
        switch (_master->getOp()) {
            case BITWISE_AND : retStr += _aSliced + " &    "    + _bSliced; break;
            case BITWISE_OR  : retStr += _aSliced + " |    "    + _bSliced; break;
            case BITWISE_XOR : retStr += _aSliced + " ^    "    + _bSliced; break;
            case BITWISE_INVR: retStr += " ~  "   + _aSliced;               break;
            case BITWISE_SHL : retStr += _aSliced + " <<   "    + _bSliced; break;
            case BITWISE_SHR : retStr += _aSliced + " >>   "    + _bSliced; break;
            case LOGICAL_AND : retStr += _aSliced + " &&   "    + _bSliced; break;
            case LOGICAL_OR  : retStr += _aSliced + " ||   "    + _bSliced; break;
            case LOGICAL_NOT : retStr += " !    " + _aSliced;               break;
            case RELATION_EQ : retStr += _aSliced + " ==   "    + _bSliced; break;
            case RELATION_NEQ: retStr += _aSliced + " !=   "    + _bSliced; break;
            case RELATION_LE : retStr += _aSliced + " <    "    + _bSliced; break;
            case RELATION_LEQ: retStr += _aSliced + " <=   "    + _bSliced; break;
            case RELATION_GE : retStr += _aSliced + " >    "    + _bSliced; break;
            case RELATION_GEQ: retStr += _aSliced + " >=   "    + _bSliced; break;
            case ARITH_PLUS  : retStr += _aSliced + " +    "    + _bSliced; break;
            case ARITH_MINUS : retStr += _aSliced + " -    "    + _bSliced; break;
            case ARITH_MUL   : retStr += _aSliced + " *    "    + _bSliced; break;
            case ARITH_DIV   : retStr += _aSliced + " /    "    + _bSliced; break;
            case ARITH_DIVR  : retStr += _aSliced + " %    "    + _bSliced; break;
            case EXTEND_BIT  : retStr += _aSliced + ".extend<"+ std::to_string(desSize)+">()";   break;
            case ASSIGN      : retStr += _aSliced                      ; break;
            case OP_DUMMY:
            case LOGIC_OP_COUNT: break;
        }

        retStr += ";\n";
        retStr += "     }\n";

        return retStr;

    }


}