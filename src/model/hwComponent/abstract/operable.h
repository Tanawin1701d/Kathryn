//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_OPERABLE_H
#define KATHRYN_OPERABLE_H

#include<memory>
#include "Slice.h"
#include "operation.h"

namespace kathryn {

    /**
     * Operable represents hardware component that can be drive the
     * logic value
     *
     * */
    class expression;
    class Operable{
    private:

    public:
        explicit Operable() = default;
        /** bitwise operators*/
        virtual expression& operator &  (Operable& b);
        virtual expression& operator |  (Operable& b);
        virtual expression& operator ^  (Operable& b);
        virtual expression& operator ~  ();
        virtual expression& operator << (Operable& b);
        virtual expression& operator >> (Operable& b);
        /** logical operators*/
        virtual expression& operator && (Operable& b);
        virtual expression& operator || (Operable& b);
        virtual expression& operator !  ();
        /** relational operator*/
        virtual expression& operator == (Operable& b);
        virtual expression& operator != (Operable& b);
        virtual expression& operator <  (Operable& b);
        virtual expression& operator <= (Operable& b);
        virtual expression& operator >  (Operable& b);
        virtual expression& operator >= (Operable& b);
        /** arithmetic operators*/
        virtual expression& operator +  (Operable& b);
        virtual expression& operator -  (Operable& b);
        virtual expression& operator *  (Operable& b);
        virtual expression& operator /  (Operable& b);
        virtual expression& operator %  (Operable& b);
        /** todo for now self assign operation such as += is not permit */

        /** due to slice operable maybe change*/
        virtual Slice getOperableSlice() = 0;
        virtual Operable& getExactOperable() = 0;
    };


}

#endif //KATHRYN_OPERABLE_H
