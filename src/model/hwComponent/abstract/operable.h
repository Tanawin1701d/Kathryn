//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_OPERABLE_H
#define KATHRYN_OPERABLE_H

#define opr Operable

#include<memory>
#include "model/debugger/modelDebugger.h"

#include "Slice.h"
#include "operation.h"
#include "identifiable.h"
#include "makeComponent.h"
#include "model/hwComponent/abstract/modelMode.h"


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
        ValRepBase cachedRep;
        bool isCacheRepInit = false;

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
                static int  balanceSize(Operable& a, Operable& b);
        virtual expression& extB(int desSize);
        virtual Operable&   uext(int desSize);
                Operable*   uextIfSizeNotEq(int desSize);
                std::pair<Operable&, Operable&>
                            uextToBalanceSize(Operable& a, Operable& b);


        virtual Operable&   sext(int desSize);
                Operable*   sextIfSizeNotEq(int desSize);
                std::pair<Operable&, Operable&>
                            sextToBalanceSize(Operable& a, Operable& b);




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
        virtual Slice           getOperableSlice() const = 0;
        [[nodiscard]]
        virtual Operable&       getExactOperable () const = 0;

        virtual Operable*       doSlice(Slice sl) = 0; //// sl is relative
                Operable&       sl(int start, int stop);
                Operable&       sl(int start);
        /** please remind this is a copy not reference value*/
        virtual LogicSimEngine* getLogicSimEngineFromOpr() = 0;
        virtual LogicGenBase*   getLogicGenBase() = 0;

        explicit operator ull();
        explicit operator ValRepBase();
        ValRepBase v();
        void initValRep(const ValRepBase& vrb);


        /**downcasting*/
        virtual Identifiable*   castToIdent() = 0;
        Operable&               getMatchOperable(ull value) const;

        /** check logic section*/
        bool isInCheckPath = false;
        virtual Operable*       checkShortCircuit() = 0;

        /** constant value dec*/
        virtual bool            isConstOpr(){return false;}
        virtual ull             getConstOpr(){assert(false);}



    };

}

#endif //KATHRYN_OPERABLE_H
