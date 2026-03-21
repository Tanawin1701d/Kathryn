//
// Created by tanawin on 20/6/2024.
//

#include "expr_gen.h"

#include "gen/proxy_hw_comp/module/module_gen.h"
#include "model/hw_component/expression/expression.h"

namespace kathryn{

    ExprGen::ExprGen(ModuleGen*    md_gen_master,
                     expression*   master):
        LogicGenBase(md_gen_master,
                     (Assignable*) master,
                     (Identifiable*) master),
        _master(master){
        assert(_master != nullptr);
    }


    void ExprGen::route_dep(){
        if (_master->_a != nullptr){
            _routedOprA = _mdGenMaster->
            route_src_opr_to_this_module(_master->_a);
        }

        if (_master->_b != nullptr){
            _routedOprB = _mdGenMaster->
            route_src_opr_to_this_module(_master->_b);
        }
    }

    std::string ExprGen::dec_io(){
        return "";
    }

    std::string ExprGen::dec_variable(){
        Slice sl = _master->get_operable_slice();
        return "wire [" + std::to_string(sl.stop-1) +
            ": 0]" + get_opr() + ";";
    }

    std::string ExprGen::dec_op(){
        std::string ret_str;


        std::string _aSliced = get_opr_str_from_opr(_routedOprA);
        std::string _bSliced;

        int a_size = _master->_a->get_operable_slice().get_size();
        int b_size = 0;

        if (_routedOprB != nullptr){
            _bSliced = get_opr_str_from_opr(_routedOprB);
            b_size    = _master->_b->get_operable_slice().get_size();
        }

        ret_str += "assign " + get_opr() + " = ";

        switch (_master->get_op()) {
            /** bitwise operators*/
            case BITWISE_AND : {assert(a_size == b_size); ret_str += _aSliced + " &    "    + _bSliced; break;}
            case BITWISE_OR  : {assert(a_size == b_size); ret_str += _aSliced + " |    "    + _bSliced; break;}
            case BITWISE_XOR : {assert(a_size == b_size); ret_str += _aSliced + " ^    "    + _bSliced; break;}
            case BITWISE_INVR: {                        ret_str += "~"      + _aSliced;               break;}
            case BITWISE_SHL : {                        ret_str += _aSliced + " <<   "    + _bSliced; break;}
            case BITWISE_SHR : {                        ret_str += _aSliced + " >>   "    + _bSliced; break;}
            /** logical operator*/
            case LOGICAL_AND : {assert(a_size == b_size); ret_str += _aSliced + " &&   "    + _bSliced; break;}
            case LOGICAL_OR  : {assert(a_size == b_size); ret_str += _aSliced + " ||   "    + _bSliced; break;}
            case LOGICAL_NOT : {                        ret_str += " !    " + _aSliced;               break;}
            /** relational operator*/
            case RELATION_EQ : {assert(a_size == b_size); ret_str += _aSliced + " ==   "    + _bSliced; break;}
            case RELATION_NEQ: {assert(a_size == b_size); ret_str += _aSliced + " !=   "    + _bSliced; break;}
            case RELATION_LE : {assert(a_size == b_size); ret_str += _aSliced + " <    "    + _bSliced; break;}
            case RELATION_LEQ: {assert(a_size == b_size); ret_str += _aSliced + " <=   "    + _bSliced; break;}
            case RELATION_GE : {assert(a_size == b_size); ret_str += _aSliced + " >    "    + _bSliced; break;}
            case RELATION_GEQ: {assert(a_size == b_size); ret_str += _aSliced + " >=   "    + _bSliced; break;}
            case RELATION_SLT: {assert(a_size == b_size); ret_str += cvt_to_sign_sig(_aSliced) + " <   " + cvt_to_sign_sig(_bSliced); break;} /// change to signed integer
            case RELATION_SGT: {assert(a_size == b_size); ret_str += cvt_to_sign_sig(_aSliced) + " >   " + cvt_to_sign_sig(_bSliced); break;} /// change to signed integer
            /** arithmetic operator*/
            case ARITH_PLUS  : {assert(a_size == b_size); ret_str += _aSliced + "   +  "    + _bSliced; break;}
            case ARITH_MINUS : {assert(a_size == b_size); ret_str += _aSliced + "   -  "    + _bSliced; break;}
            case ARITH_MUL   : {assert(a_size == b_size); ret_str += _aSliced + "   *  "    + _bSliced; break;}
            case ARITH_DIV   : {assert(a_size == b_size); ret_str += "(" + _bSliced + " == 0) ? 0: " +
                                                                  "(" + _aSliced + " / "          + _bSliced + ")"; break;}
            case ARITH_DIVR  : {assert(a_size == b_size); ret_str += "(" + _bSliced + " == 0) ? 0: " +
                                                                  "(" + _aSliced + " % "          + _bSliced + ")"; break;}
            case EXTEND_BIT  :{
                // int des_size = _master->get_slice().get_size();
                // ull des_mask = des_size == bit_size_of_ull ? -1: ((((ull)1) << des_size) - 1);

                //ret_str += _aSliced + " ? " + std::to_string(des_mask) + " : 0";  break;
                ret_str += _aSliced + " ? -1 : 0 ";  break;
            }
            case ASSIGN      :{  ret_str += _aSliced; break;}
            case OP_DUMMY:
            case LOGIC_OP_COUNT: break;
        }

        return ret_str + ";";

    }

    std::string ExprGen::cvt_to_sign_sig(std::string src_str){
        return "$signed(" + src_str + ")";
    }

}