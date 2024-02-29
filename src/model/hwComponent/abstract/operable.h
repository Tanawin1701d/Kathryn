//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_OPERABLE_H
#define KATHRYN_OPERABLE_H

#include<memory>
#include "Slice.h"
#include "operation.h"
#include "identifiable.h"
#include "makeComponent.h"
#include "model/hwComponent/abstract/modelMode.h"
#include "model/simIntf/modelSimInterface.h"

namespace kathryn {

    /**
     * Operable represents hardware component that can be drive the
     * logic value
     *
     * */
    template<typename T>
    class LogicComp;
    class expression;
    struct AssignMeta;
    class Operable{

    public:
        explicit Operable() = default;
        virtual ~Operable() = default;
        /** bitwise operators*/
        virtual expression& operator &  (const Operable& b);
        virtual expression& operator |  (const Operable& b);
        virtual expression& operator ^  (const Operable& b);
        virtual expression& operator ~  ();
        virtual expression& operator << (const Operable& b);
        virtual expression& operator >> (const Operable& b);
        /** logical operators*/
        virtual expression& operator && (const Operable& b);
        virtual expression& operator || (const Operable& b);
        virtual expression& operator !  ();
        /** relational operator*/
        virtual expression& operator == (const Operable& b);
        virtual expression& operator != (const Operable& b);
        virtual expression& operator <  (const Operable& b);
        virtual expression& operator <= (const Operable& b);
        virtual expression& operator >  (const Operable& b);
        virtual expression& operator >= (const Operable& b);
        /** arithmetic operators*/
        virtual expression& operator +  (const Operable& b);
        virtual expression& operator -  (const Operable& b);
        virtual expression& operator *  (const Operable& b);
        virtual expression& operator /  (const Operable& b);
        virtual expression& operator %  (const Operable& b);
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

        virtual Operable*       doSlice(Slice sl) = 0; //// sl is abs value

        virtual Simulatable*    getSimItf() = 0;
        virtual RtlValItf*      getRtlValItf() = 0;
        /** please remind this is a copy not reference value*/
        ValRep  getSlicedCurValue(); ///// get simvalue with sliced to match current opr

        /**downcasting*/
        virtual Identifiable*   castToIdent() = 0;
        [[maybe_unused]] ///// sv is legacy now we can use operator = to enable simultion value
        virtual ValRep&         sv() = 0;

        Operable& getMatchOperable(ull value) const;

        bool isInCheckPath = false;
        virtual Operable* checkShortCircuit() = 0;



        /** convert to value for simulation*/

        explicit operator ull(){
            std::cout << "get simvalue is used" << std::endl;
            assert(getAssignMode() == AM_SIM);
            assert(getRtlValItf()->isCurValSim());
            assert(getRtlValItf()->getCurVal().getValArrSize() == 1);
            return getRtlValItf()->getCurVal().getVal()[0];
        }
        explicit operator ValRep&(){
            assert(getAssignMode() == AM_SIM);
            return getRtlValItf()->getCurVal();
        }





    };

}

#endif //KATHRYN_OPERABLE_H
