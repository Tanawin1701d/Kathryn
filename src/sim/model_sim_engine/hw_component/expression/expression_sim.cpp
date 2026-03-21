//
// Created by tanawin on 18/7/2024.
//

#include "expression_sim.h"
#include "model/hw_component/expression/expression.h"
#include "sim/model_sim_engine/hw_component/abstract/gen_helper.h"


namespace kathryn{


    /**
     *
     * expression sim
     * */

    expression_sim_engine::expression_sim_engine(expression* master,
                                           VCD_SIG_TYPE sig_type):
            LogicSimEngine(master, master, master, sig_type, false, 0),
            _master(master){
        assert(master != nullptr);
    }

    void expression_sim_engine::proxy_build_init(){

        /////// add dependency
        assert(_master->_a != nullptr);
        dep.push_back(_master->_a->get_logic_sim_engine_from_opr_ptr());
        if (_master->_b != nullptr){
            dep.push_back(_master->_b->get_logic_sim_engine_from_opr_ptr());
        }

    }

    void expression_sim_engine::create_global_variable(CbBaseCxx& cb){
        if (_reqGlobDec){
            cb.add_st(get_val_rep().build_var());
        }
    }


    void expression_sim_engine::create_local_variable(CbBaseCxx& cb){
        if (!_reqGlobDec){
            cb.add_st(get_val_rep().build_var());
        }
    }

    void expression_sim_engine::create_op(CbBaseCxx& cb){


        std::string ret_str;

        bool need_close_bracket = true;
        ValR des_opr = get_val_rep();

        assert(_asb   ->get_assign_slice().get_size());
        assert(_master->_a->get_operable_slice().check_valid_slice());


        /****/
        ValR _aSliced = get_sliced_src_opr_from_opr(_master->_a, getMatchSVT_ALL(_master->_a));
        ValR _bSliced;
        int a_size = _master->_a->get_operable_slice().get_size();
        assert(a_size == _aSliced._size);
        int b_size = 0;
        if (_master->_b != nullptr){
            _bSliced = get_sliced_src_opr_from_opr(_master->_b, getMatchSVT_ALL(_master->_b));
            b_size    = _master->_b->get_operable_slice().get_size();
            assert(b_size == _bSliced._size);
        }

        int des_size = _master->get_slice().get_size();
        std::string des_size_str = std::to_string(des_size);

        //////// if this variable have only x bit we must set only x bit other 64-x must be exterminated
        ///std::string des_mask = cvt_num2_hex_str(create_mask(_master->get_slice()));
        ValR operated_var;

        ////// all operand expect to be equal
        switch (_master->get_op()) {
            case BITWISE_AND : {assert(a_size == b_size);   operated_var = _aSliced & _bSliced;               break;}
            case BITWISE_OR  : {assert(a_size == b_size);   operated_var = _aSliced | _bSliced;               break;}
            case BITWISE_XOR : {assert(a_size == b_size);   operated_var = _aSliced ^ _bSliced;               break;}
            case BITWISE_INVR: {assert(a_size == des_size); operated_var = (~_aSliced).enforce_size();         break;}
            case BITWISE_SHL : {assert(a_size == des_size); operated_var = (_aSliced<<_bSliced).enforce_size();break;}
            case BITWISE_SHR : {assert(a_size == des_size); operated_var = (_aSliced>>_bSliced).enforce_size();break;}
            case LOGICAL_AND : {assert(a_size == b_size);   operated_var = _aSliced && _bSliced;              break;}
            case LOGICAL_OR  : {assert(a_size == b_size);   operated_var = _aSliced||_bSliced;                break;}
            case LOGICAL_NOT : {                      ;   operated_var = !_aSliced;                         break;}
            case RELATION_EQ : {assert(a_size == b_size);   operated_var = _aSliced == _bSliced;              break;}
            case RELATION_NEQ: {assert(a_size == b_size);   operated_var = _aSliced != _bSliced;              break;}
            case RELATION_LE : {assert(a_size == b_size);   operated_var = _aSliced <  _bSliced;              break;}
            case RELATION_LEQ: {assert(a_size == b_size);   operated_var = _aSliced <= _bSliced;              break;}
            case RELATION_GE : {assert(a_size == b_size);   operated_var = _aSliced >  _bSliced;              break;}
            case RELATION_GEQ: {assert(a_size == b_size);   operated_var = _aSliced >= _bSliced;              break;}
            case RELATION_SLT: {assert(a_size == b_size);   operated_var = _aSliced.slt(_bSliced);            break;}
            case RELATION_SGT: {assert(a_size == b_size);   operated_var = _aSliced.sgt(_bSliced);            break;}
            case ARITH_PLUS  :{
                assert(a_size == b_size);
                operated_var = (_aSliced + _bSliced).enforce_size();
                break;
            }
            case ARITH_MINUS :{
                assert(a_size == b_size);
                operated_var = (_aSliced - _bSliced).enforce_size();
                break;
            }
            case ARITH_MUL   :{
                assert(a_size == b_size);
                operated_var = (_aSliced * _bSliced).enforce_size();
                break;
            }
            case ARITH_DIV   :{
                assert(a_size == b_size);
                operated_var = (_aSliced / _bSliced).enforce_size();
                break;
            }
            case ARITH_DIVR  :{
                assert(a_size == b_size);
                operated_var = (_aSliced % _bSliced).enforce_size();
                break;
            }
            case EXTEND_BIT  :{
                assert(a_size == 1);
                operated_var = _aSliced.ext(des_size);
                break;
            }
            case ASSIGN      :{  operated_var = _aSliced; break;}
            case OP_DUMMY:
            case LOGIC_OP_COUNT: break;
        }

        if ( (_master->get_op() == ARITH_DIV) || (_master->get_op() == ARITH_DIVR)){ ////// it have to check the value

            //////// make zero
            ValR x(getValR_Type(), b_size, std::to_string(_initVal)+CXX_ULL_SUFFIX);
            /////// due to ull string init we must cast first
            ValR zero = x.cast_base(x._valType, x._size);

            //////// create the conditional block
            CbIfCxx& cb_if_block   = cb.add_if( (_bSliced == zero).to_string());
            CbIfCxx& cb_else_block = cb_if_block.add_elif("");

            cb_if_block.add_st(des_opr.eq(zero).to_string());   //// the all source and size of thedestination should be equal
            cb_else_block.add_st(des_opr.eq(operated_var).to_string());
        }else{
            ValR result_opr = des_opr.eq(operated_var);
            cb.add_st(result_opr.to_string());
        }



    }

}
