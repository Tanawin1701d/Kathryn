//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_OPERABLE_H
#define KATHRYN_OPERABLE_H

#define opr Operable

#include "memory"
#include "model/debugger/model_debugger.h"

#include "slice.h"
#include "operation.h"
#include "identifiable.h"
#include "make_component.h"
#include "model/hw_component/abstract/model_mode.h"


namespace kathryn {

    /**
     * Operable represents hardware component that can be drive the
     * logic value
     *
     * */
    // template<typename T>
    // class LogicComp;
    class expression;
    class LogicSimEngine;
    class LogicGenBase;
    struct AssignMeta;

    class Operable;
    typedef std::vector<std::reference_wrapper<Operable>> Oprs;

    class Operable{
    protected:
        ValRepBase cached_rep;
        bool is_cache_rep_init = false;

    public:
        explicit Operable() = default;
        virtual ~Operable() = default;
        /** bitwise operators*/
        virtual expression& operator &  ( Operable& b);
        virtual expression& operator |  (Operable& b);
        virtual expression& operator ^  ( Operable& b);
        virtual expression& operator ~  ();
        virtual expression& operator << ( Operable& b);
        virtual expression& operator >> ( Operable& b);
        /** logical operators*/
        virtual expression& operator && ( Operable& b);
        virtual expression& operator || ( Operable& b);
        virtual expression& operator !  ();
        /** relational operator*/
        virtual expression& operator == ( Operable& b);
        virtual expression& operator != ( Operable& b);
        virtual expression& operator <  ( Operable& b);
        virtual expression& operator <= ( Operable& b);
        virtual expression& operator >  ( Operable& b);
        virtual expression& operator >= ( Operable& b);
        virtual expression& slt         ( Operable& b); /// sign less than
        virtual expression& sgt         ( Operable& b); /// sign greater than
        /** arithmetic operators*/
        virtual expression& operator +  ( Operable& b);
        virtual expression& operator -  ( Operable& b);
        virtual expression& operator *  ( Operable& b);
        virtual expression& operator /  ( Operable& b);
        virtual expression& operator %  ( Operable& b);
        /** extend bit*/
                static int  balance_size(Operable& a, Operable& b);
        virtual expression& ext_b(int des_size);
        virtual Operable&   uext(int des_size);
                Operable*   uext_if_size_not_eq(int des_size);
                std::pair<Operable&, Operable&>
                            uext_to_balance_size(Operable& a, Operable& b);


        virtual Operable&   sext(int des_size);
                Operable*   sext_if_size_not_eq(int des_size);
                std::pair<Operable&, Operable&>
                            sext_to_balance_size(Operable& a, Operable& b);




        /** todo for now self assign operation such as += is not permit */

        /**
         *
         * rhs is match ull size
         *
         * */
        virtual expression& operator &  (ull b);
        virtual expression& operator |  (ull b);
        virtual expression& operator ^  (ull b);
        virtual expression& operator << (ull b);
        virtual expression& operator >> (ull b);
        /** logical operators*/
        virtual expression& operator && (ull b);
        virtual expression& operator || (ull b);
        /** relational operator*/
        virtual expression& operator == (ull b);
        virtual expression& operator != (ull b);
        virtual expression& operator <  (ull b);
        virtual expression& operator <= (ull b);
        virtual expression& operator >  (ull b);
        virtual expression& operator >= (ull b);
        virtual expression& slt         (ull b);
        virtual expression& sgt         (ull b);
        /** arithmetic operators*/
        virtual expression& operator +  (ull b);
        virtual expression& operator -  (ull b);
        virtual expression& operator *  (ull b);
        virtual expression& operator /  (ull b);
        virtual expression& operator %  (ull b);


        /** due to slice operable maybe change*/
        [[nodiscard]]
        virtual Slice           get_operable_slice() const = 0;
        [[nodiscard]]
        virtual Operable&       get_exact_operable () const = 0;

        virtual Operable*       do_slice(Slice sl) = 0; //// sl is relative
                Operable&       sl(int start, int stop);
                Operable&       sl(int start);
        /** please remind this is a copy not reference value*/
        virtual LogicSimEngine*get_logic_sim_engine_from_opr_ptr() = 0;
        virtual LogicGenBase*   get_logic_gen_ptrBase() = 0;

        explicit operator ull();
        explicit operator ValRepBase();
        ValRepBase v();
        void init_val_rep(const ValRepBase& vrb);


        /**downcasting*/
        virtual Identifiable*   cast_to_ident() = 0;
        Operable&               get_match_operable(ull value) const;

        /** check logic section*/
        bool is_in_check_path = false;
        virtual Operable*       check_short_circuit() = 0;

        /** constant value dec*/
        virtual bool            is_const_opr(){return false;}
        virtual ull             get_const_opr(){assert(false);}



    };

}

#endif //KATHRYN_OPERABLE_H
