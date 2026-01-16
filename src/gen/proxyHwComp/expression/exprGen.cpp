//
// Created by tanawin on 20/6/2024.
//

#include "exprGen.h"

#include "gen/proxyHwComp/module/moduleGen.h"
#include "model/hwComponent/expression/expression.h"

namespace kathryn{

    ExprGen::ExprGen(ModuleGen*    mdGenMaster,
                     expression*   master):
        LogicGenBase(mdGenMaster,
                     (Assignable*) master,
                     (Identifiable*) master),
        _master(master){
        assert(_master != nullptr);
    }


    void ExprGen::routeDep(){
        if (_master->_a != nullptr){
            _routedOprA = _mdGenMaster->
            routeSrcOprToThisModule(_master->_a);
        }

        if (_master->_b != nullptr){
            _routedOprB = _mdGenMaster->
            routeSrcOprToThisModule(_master->_b);
        }
    }

    std::string ExprGen::decIo(){
        return "";
    }

    std::string ExprGen::decVariable(){
        Slice sl = _master->getOperableSlice();
        return "wire [" + std::to_string(sl.stop-1) +
            ": 0]" + getOpr() + ";";
    }

    std::string ExprGen::decOp(){
        std::string retStr;


        std::string _aSliced = getOprStrFromOpr(_routedOprA);
        std::string _bSliced;

        int aSize = _master->_a->getOperableSlice().getSize();
        int bSize = 0;

        if (_routedOprB != nullptr){
            _bSliced = getOprStrFromOpr(_routedOprB);
            bSize    = _master->_b->getOperableSlice().getSize();
        }

        retStr += "assign " + getOpr() + " = ";

        switch (_master->getOp()) {
            /** bitwise operators*/
            case BITWISE_AND : {assert(aSize == bSize); retStr += _aSliced + " &    "    + _bSliced; break;}
            case BITWISE_OR  : {assert(aSize == bSize); retStr += _aSliced + " |    "    + _bSliced; break;}
            case BITWISE_XOR : {assert(aSize == bSize); retStr += _aSliced + " ^    "    + _bSliced; break;}
            case BITWISE_INVR: {                        retStr += "~"      + _aSliced;               break;}
            case BITWISE_SHL : {                        retStr += _aSliced + " <<   "    + _bSliced; break;}
            case BITWISE_SHR : {                        retStr += _aSliced + " >>   "    + _bSliced; break;}
            /** logical operator*/
            case LOGICAL_AND : {assert(aSize == bSize); retStr += _aSliced + " &&   "    + _bSliced; break;}
            case LOGICAL_OR  : {assert(aSize == bSize); retStr += _aSliced + " ||   "    + _bSliced; break;}
            case LOGICAL_NOT : {                        retStr += " !    " + _aSliced;               break;}
            /** relational operator*/
            case RELATION_EQ : {assert(aSize == bSize); retStr += _aSliced + " ==   "    + _bSliced; break;}
            case RELATION_NEQ: {assert(aSize == bSize); retStr += _aSliced + " !=   "    + _bSliced; break;}
            case RELATION_LE : {assert(aSize == bSize); retStr += _aSliced + " <    "    + _bSliced; break;}
            case RELATION_LEQ: {assert(aSize == bSize); retStr += _aSliced + " <=   "    + _bSliced; break;}
            case RELATION_GE : {assert(aSize == bSize); retStr += _aSliced + " >    "    + _bSliced; break;}
            case RELATION_GEQ: {assert(aSize == bSize); retStr += _aSliced + " >=   "    + _bSliced; break;}
            case RELATION_SLT: {assert(aSize == bSize); retStr += cvtToSignSig(_aSliced) + " <   " + cvtToSignSig(_bSliced); break;} /// change to signed integer
            case RELATION_SGT: {assert(aSize == bSize); retStr += cvtToSignSig(_aSliced) + " >   " + cvtToSignSig(_bSliced); break;} /// change to signed integer
            /** arithmetic operator*/
            case ARITH_PLUS  : {assert(aSize == bSize); retStr += _aSliced + "   +  "    + _bSliced; break;}
            case ARITH_MINUS : {assert(aSize == bSize); retStr += _aSliced + "   -  "    + _bSliced; break;}
            case ARITH_MUL   : {assert(aSize == bSize); retStr += _aSliced + "   *  "    + _bSliced; break;}
            case ARITH_DIV   : {assert(aSize == bSize); retStr += _aSliced + "   /  "    + _bSliced; break;}
            case ARITH_DIVR  : {assert(aSize == bSize); retStr += _aSliced + "   %  "    + _bSliced; break;}
            case EXTEND_BIT  :{
                // int desSize = _master->getSlice().getSize();
                // ull desMask = desSize == bitSizeOfUll ? -1: ((((ull)1) << desSize) - 1);

                //retStr += _aSliced + " ? " + std::to_string(desMask) + " : 0";  break;
                retStr += _aSliced + " ? -1 : 0 ";  break;
            }
            case ASSIGN      :{  retStr += _aSliced; break;}
            case OP_DUMMY:
            case LOGIC_OP_COUNT: break;
        }

        return retStr + ";";

    }

    std::string ExprGen::cvtToSignSig(std::string srcStr){
        return "$signed(" + srcStr + ")";
    }

}