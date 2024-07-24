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

    void expressionSimEngine::createGlobalVariable(CbBaseCxx& cb){
        if (_reqGlobDec){
            SIM_VALREP_TYPE svt = getValR_Type();
            std::string typeStr = SVT_toType(svt);
            cb.addSt(typeStr +" " + getVarName());
        }
    }


    void expressionSimEngine::createLocalVariable(CbBaseCxx& cb){
        if (!_reqGlobDec){
            SIM_VALREP_TYPE svt     = getValR_Type();
            std::string     typeStr = SVT_toType(svt);
            cb.addSt(typeStr +" " + getVarName());
        }
    }

    void expressionSimEngine::createOp(CbBaseCxx& cb){


        std::string retStr;

        bool needCloseBracket = true;
        retStr += getVarName() + " = (";

        assert(_asb   ->getAssignSlice().getSize());
        assert(_master->_a->getOperableSlice().checkValidSlice());


        /****/


        std::string _aSliced = getSlicedSrcOprFromOpr(_master->_a, getMatchSVT(_master->_a));
        std::string _bSliced;
        int aSize = _master->_a->getOperableSlice().getSize();
        int bSize = 0;

        if (_master->_b != nullptr){
            _bSliced = getSlicedSrcOprFromOpr(_master->_b, getMatchSVT(_master->_b));
            bSize    = _master->_b->getOperableSlice().getSize();
        }

        int desSize = _master->getSlice().getSize();
        std::string desSizeStr = std::to_string(desSize);

        //////// if this variable have only x bit we must set only x bit other 64-x must be exterminated
        std::string desMask = cvtNum2HexStr(createMask(_master->getSlice()));

        ////// all operand expect to be equal
        switch (_master->getOp()) {
            case BITWISE_AND : {assert(aSize == bSize);   retStr += _aSliced + " &    "    + _bSliced; break;}
            case BITWISE_OR  : {assert(aSize == bSize);   retStr += _aSliced + " |    "    + _bSliced; break;}
            case BITWISE_XOR : {assert(aSize == bSize);   retStr += _aSliced + " ^    "    + _bSliced; break;}
            case BITWISE_INVR: {assert(aSize == desSize); retStr += "(~  "   + _aSliced + ").fixSize(" + desSizeStr  + ")";break;}
            case BITWISE_SHL : {assert(aSize == desSize); retStr += "( "     + _aSliced + " << " + _bSliced + ").fixSize(" + desSizeStr + ")"; break;}
            case BITWISE_SHR : {assert(aSize == desSize); retStr += "( "     + _aSliced + " >> " + _bSliced + ").fixSize(" + desSizeStr + ")"; break;}
            case LOGICAL_AND : {assert(aSize == bSize);   retStr += _aSliced + " &&   "    + _bSliced; break;}
            case LOGICAL_OR  : {assert(aSize == bSize);   retStr += _aSliced + " ||   "    + _bSliced; break;}
            case LOGICAL_NOT : {                      ;   retStr += " !"     + _aSliced;               break;}
            case RELATION_EQ : {assert(aSize == bSize);   retStr += _aSliced + " ==   "    + _bSliced; break;}
            case RELATION_NEQ: {assert(aSize == bSize);   retStr += _aSliced + " !=   "    + _bSliced; break;}
            case RELATION_LE : {assert(aSize == bSize);   retStr += _aSliced + " <    "    + _bSliced; break;}
            case RELATION_LEQ: {assert(aSize == bSize);   retStr += _aSliced + " <=   "    + _bSliced; break;}
            case RELATION_GE : {assert(aSize == bSize);   retStr += _aSliced + " >    "    + _bSliced; break;}
            case RELATION_GEQ: {assert(aSize == bSize);   retStr += _aSliced + " >=   "    + _bSliced; break;}
            case ARITH_PLUS  :{
                assert(aSize == bSize);
                retStr += "(" + _aSliced + " + " + _bSliced + ").fixSize(" + desSizeStr + ")";
                break;
            }
            case ARITH_MINUS :{
                assert(aSize == bSize);
                retStr += "(" + _aSliced + " - " + _bSliced + ").fixSize(" + desSizeStr + ")";
                break;
            }
            case ARITH_MUL   :{assert(false);}
            case ARITH_DIV   :{assert(false);}
            case ARITH_DIVR  :{assert(false);}
            case EXTEND_BIT  :{
                assert(bSize == 1);
                retStr = getVarName() + ".ext(+ " + desSizeStr +"," + _aSliced +")";
                needCloseBracket = false;
                break;
            }
            case ASSIGN      :{  retStr += _aSliced; break;}
            case OP_DUMMY:
            case LOGIC_OP_COUNT: break;
        }

        if (needCloseBracket){
            retStr += ")";
        }

        cb.addSt(retStr);

    }

}
