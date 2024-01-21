//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_OPERABLE_H
#define KATHRYN_OPERABLE_H

#include<memory>
#include "Slice.h"
#include "operation.h"
#include "identifiable.h"
#include "model/simIntf/rtlSimItf.h"

namespace kathryn {

    /**
     * Operable represents hardware component that can be drive the
     * logic value
     *
     * */
     template<typename T>
     class LogicComp;
    class expression;
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

        /** due to slice operable maybe change*/
        [[nodiscard]]
        virtual Slice           getOperableSlice() const = 0;
        [[nodiscard]]
        virtual Operable&       getExactOperable () const = 0;
        /**downcasting*/
        virtual Identifiable*   castToIdent() = 0;
        virtual RtlSimulatable* castToRtlSimItf() = 0;
        virtual ValRep&         sv() = 0;
    };


}

#endif //KATHRYN_OPERABLE_H
