//
// Created by tanawin on 29/11/2566.
//

#include "operable.h"

#include "regex"
#include "model/hw_component/expression/nest.h"

#include "model/hw_component/expression/expression.h"
#include "model/hw_component/value/value.h"


namespace kathryn {

/***
     * todo future,  we will make it handle case unequal size
     *
     * */

    /////// bl_a = balanced A
    /////// bl_b = balanced B

/** bitwise operators*/
    expression& Operable::operator&( Operable &b) {
        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
               "operable<&> get mismatch bit size");

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);
        int  des_size    = balance_size(*this, b);
        auto ret =  new expression(BITWISE_AND,
                                   &bl_a,
                                   &bl_b,
                                   des_size);
        return *ret;
    }

    expression& Operable::operator|(Operable &b) {
        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
               "operable<|> get mismatch bit size");

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);
        int  des_size    = balance_size(*this, b);
        auto ret =  new expression(BITWISE_OR,
                                   &bl_a,
                                   &bl_b,
                                   des_size);

        return *ret;
    }

    expression& Operable::operator^( Operable &b) {
        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
               "operable<^> get mismatch bit size"
        );

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);
        int  des_size    = balance_size(*this, b);

        auto ret =  new expression(BITWISE_XOR,
                                    &bl_a,
                                    &bl_b,
                                    des_size);

        return *ret;
    }

    expression& Operable::operator~() {

        auto ret =  new expression(BITWISE_INVR,
                                     this,
                                     nullptr,
                                     get_operable_slice().get_size());

        return *ret;
    }

    expression& Operable::operator<<( Operable &b) {

        mf_warn(b.get_operable_slice().get_size() <= 6,  "warning there is shift bit that greater than 64");
        auto ret =  new expression(BITWISE_SHL,
                                     this,
                                     &b,
                                     get_operable_slice().get_size());

        return *ret;
    }

    expression& Operable::operator>>( Operable &b) {

        mf_warn(b.get_operable_slice().get_size() <= 6,  "warning there is shift bit that greater than 64");
        auto ret =  new expression(BITWISE_SHR,
                                     this,
                                     &b,
                                     get_operable_slice().get_size());

        return *ret;
    }


    /** logical operators*/

    expression& Operable::operator&&( Operable &b) {
        mf_warn(get_operable_slice().get_size() == 1 &&
               b.get_operable_slice().get_size() == 1,
               "operable && got size expect to have size equal to 1");
        auto ret =  new expression(LOGICAL_AND,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator||( Operable &b) {
        mf_warn(get_operable_slice().get_size() == 1 &&
               b.get_operable_slice().get_size() == 1,
               "operable || got size expect to have size equal to 1");
        auto ret =  new expression(LOGICAL_OR,
                                     this,
                                     &b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!() {
        mf_warn(get_operable_slice().get_size() == 1,
               "operable ! got size expect to have size equal to 1");
        auto ret =  new expression(LOGICAL_NOT,
                                     this,
                                     nullptr,
                                     LOGICAL_SIZE);

        return *ret;
    }

    /** relational operator*/

    expression& Operable::operator==( Operable &b) {

        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
               "operable<==> get mismatch bit size");

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);

        auto ret =  new expression(RELATION_EQ,
                                     &bl_a,
                                     &bl_b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator!=( Operable &b) {

        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
       "operable<!=> get mismatch bit size");

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);

        auto ret =  new expression(RELATION_NEQ,
                                     &bl_a,
                                     &bl_b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<( Operable &b) {

        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
       "operable< < > get mismatch bit size");

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);

        auto ret =  new expression(RELATION_LE,
                                    &bl_a,
                                    &bl_b,
                                    LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator<=( Operable &b) {

        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
       "operable< <= > get mismatch bit size");

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);

        auto ret =  new expression(RELATION_LEQ,
                                     &bl_a,
                                     &bl_b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>( Operable &b) {

        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
       "operable< > > get mismatch bit size");

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);

        auto ret =  new expression(RELATION_GE,
                                     &bl_a,
                                     &bl_b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::operator>=( Operable &b) {

        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
       "operable< >= > get mismatch bit size");

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);

        auto ret =  new expression(RELATION_GEQ,
                                     &bl_a,
                                     &bl_b,
                                     LOGICAL_SIZE);

        return *ret;
    }

    expression& Operable::slt(Operable& b){

        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
       "operable< slt(sign less than) > get mismatch bit size");

        auto [bl_a, bl_b] = sext_to_balance_size(*this, b);

        auto ret = new expression(RELATION_SLT,
                                    &bl_a,
                                    &bl_b,
                                    LOGICAL_SIZE);
        return *ret;
    }

    expression& Operable::sgt(Operable& b){

        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
       "operable< slt(sign greater than) > get mismatch bit size");

        auto [bl_a, bl_b] = sext_to_balance_size(*this, b);

        auto ret = new expression(RELATION_SGT,
                                    &bl_a,
                                    &bl_b,
                                    LOGICAL_SIZE);
        return *ret;
    }



    /** arithmetic operators*/

    expression& Operable::operator+( Operable &b) {
        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
                 "operable<+> get mismatch bit size"
        );

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);
        auto ret =  new expression(ARITH_PLUS,
                                     &bl_a,
                                     &bl_b,
                                     get_operable_slice().get_size());
        /** size + 1 because we provide carry for expr_metas*/

        return *ret;
    }

    expression& Operable::operator-( Operable &b) {
        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
                 "operable<-> get mismatch bit size"
        );
        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);

        auto ret =  new expression(ARITH_MINUS,
                                     &bl_a,
                                     &bl_b,
                                     get_operable_slice().get_size());

        return *ret;
    }

    expression& Operable::operator*( Operable &b) {
        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
                 "operable<*> get mismatch bit size"
        );

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);
        auto ret =  new expression(ARITH_MUL,
                                     &bl_a,
                                     &bl_b,
                                     get_operable_slice().get_size());

        return *ret;
    }

    expression& Operable::operator/( Operable &b) {
        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
                 "operable</> get mismatch bit size"
        );

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);
        auto ret =  new expression(ARITH_DIV,
                                     &bl_a,
                                     &bl_b,
                                     get_operable_slice().get_size());

        return *ret;
    }

    expression& Operable::operator%( Operable &b) {
        mf_warn(get_operable_slice().get_size() == b.get_operable_slice().get_size(),
                 "operable<%> get mismatch bit size"
        );

        auto [bl_a, bl_b] = uext_to_balance_size(*this, b);
        auto ret =  new expression(ARITH_DIVR,
                                     &bl_a,
                                     &bl_b,
                                     get_operable_slice().get_size());

        return *ret;
    }


    int Operable::balance_size(Operable& a, Operable& b) {
        int a_size = a.get_operable_slice().get_size();
        int b_size = b.get_operable_slice().get_size();
        assert((a_size > 0) && (b_size > 0));
        return std::max(a_size, b_size);
    }

    expression& Operable::ext_b(int des_size){
        mf_assert(des_size > 0, "des_size must greater than 0");
        mf_assert(get_operable_slice().get_size() == 1, "src extend bit must size must eeq to 1");
        auto ret = new expression(EXTEND_BIT,this, nullptr, des_size);
        return *ret;
    }
    /////////////////////////////////
    ///// do unsign extend //////////
    /////////////////////////////////
    Operable& Operable::uext(int des_size){
        mf_assert(des_size > 0, "dessize must greater than 0");
        mf_assert(des_size > get_operable_slice().get_size(), "des_size must greathan original size");
        int ori_size = get_operable_slice().get_size();
        int remain_size = des_size - ori_size;

        auto& extend_val = make_opr_val("re_size_to" +
                               std::to_string(des_size) +
                               "from" + std::to_string(ori_size),
                               remain_size, 0);
        ///// it still work with the system stack
        nest& next_nest = make_nest_man_read_only(true, {this, &extend_val});
        return next_nest;
    }

    Operable* Operable::uext_if_size_not_eq(int des_size){
        if (des_size == get_operable_slice().get_size()){
            return this;
        }
        return &uext(des_size);
    }

    std::pair<Operable&, Operable&> Operable::uext_to_balance_size(Operable& a,
                                                                Operable& b){
        /////////// check size
        int des_size = balance_size(a, b);
        /////////// upgrade size
        Operable* upgraded_a = uext_if_size_not_eq(des_size);
        Operable* upgraded_b = b.uext_if_size_not_eq(des_size);

        return {*upgraded_a, *upgraded_b};
    }

    /////////////////////////////////
    ///// do sign extend   //////////
    /////////////////////////////////

    Operable& Operable::sext(int des_size){
        mf_assert(des_size > 0, "dessize must greater than 0");
        mf_assert(des_size > get_operable_slice().get_size(), "des_size must greathan original size");
        int ori_size = get_operable_slice().get_size();
        assert(ori_size >= 1);
        int remain_size = des_size - ori_size;

        auto extend_val = new expression(EXTEND_BIT, this->do_slice({ori_size-1, ori_size}), nullptr, remain_size);
        nest& next_nest = make_nest_man_read_only(true, {this, extend_val});
        return next_nest;
    }

    Operable* Operable::sext_if_size_not_eq(int des_size){
        if (des_size == get_operable_slice().get_size()){
            return this;
        }
        return &sext(des_size);
    }

    std::pair<Operable&, Operable&>
    Operable::sext_to_balance_size(Operable& a, Operable& b){
        /////////// check size
        int des_size = balance_size(a, b);
        /////////// upgrade size
        Operable* upgraded_a = sext_if_size_not_eq(des_size);
        Operable* upgraded_b = b.sext_if_size_not_eq(des_size);

        return {*upgraded_a, *upgraded_b};
    }


    Operable& Operable::sl(int start, int stop){
        return *do_slice({start, stop});
    }

    Operable& Operable::sl(int start){
        return *do_slice({start, start+1});
    }

    Operable& Operable::get_match_operable( ull value) const {
            make_val(opt_user_auto_val, get_operable_slice().get_size(), value);
            /** todo check bit size */
            return opt_user_auto_val;
    }

    Operable::operator ull(){
        return v().get_val();

    }
    Operable::operator ValRepBase(){
        return v();

    }

    ValRepBase Operable::v(){
        if(is_cache_rep_init){
            return cached_rep;
        }
        mf_assert(get_assign_mode() == AM_SIM, "can't retrieve data in model building mode");
        LogicSimEngine* sim_engine = get_logic_sim_engine_from_opr_ptr();
        return sim_engine->get_proxy_rep().slice(get_operable_slice().start, get_operable_slice().stop);
    }

    void Operable::init_val_rep(const ValRepBase& vrb){
        assert(!is_cache_rep_init);
        cached_rep = vrb;
        is_cache_rep_init = true;
    }






}