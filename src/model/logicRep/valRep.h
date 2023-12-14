//
// Created by tanawin on 13/12/2566.
//

#ifndef KATHRYN_VALREP_H
#define KATHRYN_VALREP_H

#include <cstdio>

namespace kathryn{

    typedef unsigned long long ull;
    /**
     * this class is used to represent value the logic while simulating or building
     * model
     * */

    /**
     *
     * shink mechanisim is used when inconsistent
     * bit occur this new class will operate with
     * new size based on shorter val representation
     */
    class ValRep{
    private:
        size_t _len = -1;
        int lenValArr = -1;
        ull*  _val = nullptr;

    public:
        explicit ValRep(int len);
        ~ValRep();

        ValRep(const ValRep& rhs);

        ValRep& operator = (const ValRep& rhs);
        ValRep& operator & (const ValRep& rhs);
        ValRep& operator | (const ValRep& rhs);
        ValRep& operator ^ (const ValRep& rhs);
        ValRep& operator ~ ();
        ValRep& operator << (const ValRep& rhs);
        ValRep& operator >> (const ValRep& rhs);

        ValRep& operator && (const ValRep& rhs);
        ValRep& operator || (const ValRep& rhs);
        ValRep& operator !  ();
        ValRep& operator == (const ValRep& rhs);
        ValRep& operator != (const ValRep& rhs);
        ValRep& operator <  (const ValRep& rhs);
        ValRep& operator <= (const ValRep& rhs);
        ValRep& operator >  (const ValRep& rhs);
        ValRep& operator >= (const ValRep& rhs);
        ValRep& operator +  (const ValRep& rhs);
        ValRep& operator -  (const ValRep& rhs);
        ValRep& operator *  (const ValRep& rhs);
        ValRep& operator /  (const ValRep& rhs);
        ValRep& operator %  (const ValRep& rhs);

    };

}

#endif //KATHRYN_VALREP_H
