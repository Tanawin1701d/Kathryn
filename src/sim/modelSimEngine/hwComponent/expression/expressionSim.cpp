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
            LogicSimEngine(master, master, master, sigType, false, 0),
            _master(master){
        assert(master != nullptr);
    }

    void expressionSimEngine::proxyBuildInit(){

        /////// add dependency
        assert(_master->_a != nullptr);
        dep.pushBack(_master->_a->getLogicSimEngineFromOprPtr());
        if (_master->_b != nullptr){
            dep.pushBack(_master->_b->getLogicSimEngineFromOprPtr());
        }

    }

    void expressionSimEngine::createGlobalVariable(CbBaseCxx& cb){
        if (_reqGlobDec){
            cb.addSt(getValRep().buildVar());
        }
    }


    void expressionSimEngine::createLocalVariable(CbBaseCxx& cb){
        if (!_reqGlobDec){
            cb.addSt(getValRep().buildVar());
        }
    }

    void expressionSimEngine::createOp(CbBaseCxx& cb){


        std::string retStr;

        bool needCloseBracket = true;
        ValR desOpr = getValRep();

        assert(_asb   ->getAssignSlice().getSize());
        assert(_master->_a->getOperableSlice().checkValidSlice());


        /****/
        ValR _aSliced = getSlicedSrcOprFromOpr(_master->_a, getMatchSVT_ALL(_master->_a));
        ValR _bSliced;
        int aSize = _master->_a->getOperableSlice().getSize();
        assert(aSize == _aSliced._size);
        int bSize = 0;
        if (_master->_b != nullptr){
            _bSliced = getSlicedSrcOprFromOpr(_master->_b, getMatchSVT_ALL(_master->_b));
            bSize    = _master->_b->getOperableSlice().getSize();
            assert(bSize == _bSliced._size);
        }

        int desSize = _master->getSlice().getSize();
        std::string desSizeStr = std::toString(desSize);

        //////// if this variable have only x bit we must set only x bit other 64-x must be exterminated
        ///std::string desMask = cvtNum2HexStr(createMask(_master->getSlice()));
        ValR operatedVar;

        ////// all operand expect to be equal
        switch (_master->getOp()) {
            case BITWISE_AND : {assert(aSize == bSize);   operatedVar = _aSliced & _bSliced;               break;}
            case BITWISE_OR  : {assert(aSize == bSize);   operatedVar = _aSliced | _bSliced;               break;}
            case BITWISE_XOR : {assert(aSize == bSize);   operatedVar = _aSliced ^ _bSliced;               break;}
            case BITWISE_INVR: {assert(aSize == desSize); operatedVar = (~_aSliced).enforceSize();         break;}
            case BITWISE_SHL : {assert(aSize == desSize); operatedVar = (_aSliced<<_bSliced).enforceSize();break;}
            case BITWISE_SHR : {assert(aSize == desSize); operatedVar = (_aSliced>>_bSliced).enforceSize();break;}
            case LOGICAL_AND : {assert(aSize == bSize);   operatedVar = _aSliced && _bSliced;              break;}
            case LOGICAL_OR  : {assert(aSize == bSize);   operatedVar = _aSliced||_bSliced;                break;}
            case LOGICAL_NOT : {                      ;   operatedVar = !_aSliced;                         break;}
            case RELATION_EQ : {assert(aSize == bSize);   operatedVar = _aSliced == _bSliced;              break;}
            case RELATION_NEQ: {assert(aSize == bSize);   operatedVar = _aSliced != _bSliced;              break;}
            case RELATION_LE : {assert(aSize == bSize);   operatedVar = _aSliced <  _bSliced;              break;}
            case RELATION_LEQ: {assert(aSize == bSize);   operatedVar = _aSliced <= _bSliced;              break;}
            case RELATION_GE : {assert(aSize == bSize);   operatedVar = _aSliced >  _bSliced;              break;}
            case RELATION_GEQ: {assert(aSize == bSize);   operatedVar = _aSliced >= _bSliced;              break;}
            case RELATION_SLT: {assert(aSize == bSize);   operatedVar = _aSliced.slt(_bSliced);            break;}
            case RELATION_SGT: {assert(aSize == bSize);   operatedVar = _aSliced.sgt(_bSliced);            break;}
            case ARITH_PLUS  :{
                assert(aSize == bSize);
                operatedVar = (_aSliced + _bSliced).enforceSize();
                break;
            }
            case ARITH_MINUS :{
                assert(aSize == bSize);
                operatedVar = (_aSliced - _bSliced).enforceSize();
                break;
            }
            case ARITH_MUL   :{
                assert(aSize == bSize);
                operatedVar = (_aSliced * _bSliced).enforceSize();
                break;
            }
            case ARITH_DIV   :{
                assert(aSize == bSize);
                operatedVar = (_aSliced / _bSliced).enforceSize();
                break;
            }
            case ARITH_DIVR  :{
                assert(aSize == bSize);
                operatedVar = (_aSliced % _bSliced).enforceSize();
                break;
            }
            case EXTEND_BIT  :{
                assert(aSize == 1);
                operatedVar = _aSliced.ext(desSize);
                break;
            }
            case ASSIGN      :{  operatedVar = _aSliced; break;}
            case OP_DUMMY:
            case LOGIC_OP_COUNT: break;
        }

        if ( (_master->getOp() == ARITH_DIV) || (_master->getOp() == ARITH_DIVR)){ ////// it have to check the value

            //////// make zero
            ValR x(getValR_Type(), bSize, std::toString(_initVal)+CXX_ULL_SUFFIX);
            /////// due to ull string init we must cast first
            ValR zero = x.castBase(x._valType, x._size);

            //////// create the conditional block
            CbIfCxx& cbIfBlock   = cb.addIf( (_bSliced == zero).toString());
            CbIfCxx& cbElseBlock = cbIfBlock.addElif("");

            cbIfBlock.addSt(desOpr.eq(zero).toString());   //// the all source and size of thedestination should be equal
            cbElseBlock.addSt(desOpr.eq(operatedVar).toString());
        }else{
            ValR resultOpr = desOpr.eq(operatedVar);
            cb.addSt(resultOpr.toString());
        }



    }

}
