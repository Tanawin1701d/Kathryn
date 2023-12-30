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
        int   _len             = -1;
        int   _valSize         = -1; //// size of array that contain ull
        ull*  _val             = nullptr;
        const int bitSizeOfUll = sizeof(ull) << 3;

    public:
        explicit ValRep(int len);

        ~ValRep();

        ValRep(const ValRep& rhs);

        int getLen() const{
            return _len;
        }
        int getValArrSize() const{
            return _valSize;
        }

        ull* getVal() const{
            return _val;
        }

        ValRep getZeroExtend(int targetSize);

        inline bool checkEqualBit(const ValRep& rhs) const{
            return _len == rhs._len;
        }

        ValRep bwOperator(const ValRep& rhs,
                          const std::function<ull(ull, ull)>& operation);
        ValRep eqOperator(const ValRep& rhs, bool checkEq);
        bool   getLogicalValue() const;
        ValRep logicalOperator(const ValRep& rhs,
                               const std::function<bool(bool, bool)>& operation) const;
        ValRep cmpOperator(const ValRep& rhs,
                           const std::function<bool(ull* a, ull* b, int size)>& operation);


        ValRep& operator = (const ValRep& rhs);

        //////// required equal bit operator
        ValRep operator & (const ValRep& rhs);
        ValRep operator | (const ValRep& rhs);
        ValRep operator ^ (const ValRep& rhs);
        ValRep operator == (const ValRep& rhs);
        ValRep operator != (const ValRep& rhs);
        //////// only one operand
        ValRep operator ~ ();

        //////// not required equal bit operator
        ////////////// but need zero extend
        ValRep operator && (const ValRep& rhs) const;
        ValRep operator || (const ValRep& rhs) const;
        ValRep operator !  () const;
        ValRep operator <  (const ValRep& rhs);
        ValRep operator <= (const ValRep& rhs);
        ValRep operator >  (const ValRep& rhs);
        ValRep operator >= (const ValRep& rhs);

        ///// not required equal bit operator
        ValRep operator +  (const ValRep& rhs);
        ValRep operator -  (const ValRep& rhs);
        ValRep operator *  (const ValRep& rhs){assert(false); return ValRep(0);};
        ValRep operator /  (const ValRep& rhs){assert(false); return ValRep(0);};
        ValRep operator %  (const ValRep& rhs){assert(false); return ValRep(0);};
        ValRep operator << (const ValRep& rhs);
        ValRep operator >> (const ValRep& rhs);


    };

}

#endif //KATHRYN_VALREP_H
