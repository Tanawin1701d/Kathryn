//
// Created by tanawin on 18/7/2024.
//

#include "expressionSim.h"
#include "model/hwComponent/expression/expression.h"
#include "sim/modelSimEngine/hwComponent/abstract/genHelper.h"


namespace kathryn{


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

    void expressionSimEngine::createLocalVariable(CbBaseCxx& cb){
        cb.addSt("ull " + getVarName());
    }

    void expressionSimEngine::createOp(CbBaseCxx& cb){


        std::string retStr;

        retStr += getVarName() + " = (";

        assert(_asb   ->getAssignSlice().getSize());
        assert(_master->_a->getOperableSlice().checkValidSlice());

        std::string _aSliced = getSlicedSrcOprFromOpr(_master->_a);
        std::string _bSliced;
        int aSize = _master->_a->getOperableSlice().getSize();
        int bSize = 0;

        if (_master->_b != nullptr){
            _bSliced = getSlicedSrcOprFromOpr(_master->_b);
            bSize    = _master->_b->getOperableSlice().getSize();
        }

        int desSize = _master->getSlice().getSize();

        //////// if this variable have only x bit we must set only x bit other 64-x must be exterminated
        std::string desMask = cvtNum2HexStr(createMask(_master->getSlice()));

        ////// all operand expect to be equal
        switch (_master->getOp()) {
            case BITWISE_AND : {assert(aSize == bSize); retStr += _aSliced + " &    "    + _bSliced; break;}
            case BITWISE_OR  : {assert(aSize == bSize); retStr += _aSliced + " |    "    + _bSliced; break;}
            case BITWISE_XOR : {assert(aSize == bSize); retStr += _aSliced + " ^    "    + _bSliced; break;}
            case BITWISE_INVR: {                        retStr += "(~  "+ _aSliced + ") & " + desMask;break;}
            case BITWISE_SHL : {                        retStr += "( "  + _aSliced + " <<   "    + _bSliced + ") & " + desMask; break;}
            case BITWISE_SHR : {                        retStr += "( "     + _aSliced + " >>   "    + _bSliced + ") & " + desMask; break;}
            case LOGICAL_AND : {assert(aSize == bSize); retStr += _aSliced + " &&   "    + _bSliced; break;}
            case LOGICAL_OR  : {assert(aSize == bSize); retStr += _aSliced + " ||   "    + _bSliced; break;}
            case LOGICAL_NOT : {                      ; retStr += " !    " + _aSliced;               break;}
            case RELATION_EQ : {assert(aSize == bSize); retStr += _aSliced + " ==   "    + _bSliced; break;}
            case RELATION_NEQ: {assert(aSize == bSize); retStr += _aSliced + " !=   "    + _bSliced; break;}
            case RELATION_LE : {assert(aSize == bSize); retStr += _aSliced + " <    "    + _bSliced; break;}
            case RELATION_LEQ: {assert(aSize == bSize); retStr += _aSliced + " <=   "    + _bSliced; break;}
            case RELATION_GE : {assert(aSize == bSize); retStr += _aSliced + " >    "    + _bSliced; break;}
            case RELATION_GEQ: {assert(aSize == bSize); retStr += _aSliced + " >=   "    + _bSliced; break;}
            case ARITH_PLUS  :{
                assert(aSize == bSize); retStr += "(" + _aSliced + " + " + _bSliced + ")";
                retStr += "  &   " + desMask; break;
            }
            case ARITH_MINUS :{
                assert(aSize == bSize);
                std::string binv = "((~  "   + _bSliced    + ") & " + desMask + ")";
                std::string ab1  = "(" + _aSliced + " + " +binv + "+ 1)";
                retStr += ab1 + " & " + desMask;
                break;
            }
            case ARITH_MUL   :{
                assert(aSize == bSize); retStr += "(" + _aSliced + " * " + _bSliced + ")";
                retStr += "  &   " + desMask;                 break;
            }
            case ARITH_DIV   :{
                assert(aSize == bSize); retStr += "(" + _aSliced + " / " + _bSliced + ")";
                retStr += "  &   " + desMask;                 break;
            }
            case ARITH_DIVR  :{
                assert(aSize == bSize); retStr += "(" + _aSliced + " % " + _bSliced + ")";
                retStr += "  &   " + desMask;                 break;
            }
            case EXTEND_BIT  :{
                retStr += _aSliced + " ? " + "((ull)" + desMask + ") : ((ull)0)";  break;
            }
            case ASSIGN      :{  retStr += _aSliced; break;}
            case OP_DUMMY:
            case LOGIC_OP_COUNT: break;
        }

        retStr += ")";

        cb.addSt(retStr);

    }

}
