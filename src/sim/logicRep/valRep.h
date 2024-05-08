//
// Created by tanawin on 13/12/2566.
//

#ifndef KATHRYN_VALREP_H
#define KATHRYN_VALREP_H

#include <functional>
#include <string>
#include <cstdio>
#include <bitset>
#include "model/hwComponent/abstract/Slice.h"

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
        int   _len             = -1; //// userDefine size
        int   _valSize         = -1; //// size of array that contain ull
#ifdef NOTEXCEED64
        ull   _val             = 0;
#else
        ull*  _val             = nullptr;
#endif

    public:
        static const int bitSizeOfUll = sizeof(ull) << 3;
        explicit  ValRep() = default;
        explicit ValRep(int len);

        ~ValRep();

        ValRep(const ValRep& rhs);

        int getLen() const{
            return _len;
        }

        int getValArrSize() const{return _valSize;}

        ull* getValPtr();
        ull  getVal64() const;
        /** build new valrep that have bigger than cur valrep the exceed bit wil
         * be assigned to 0
         * */
         operator ull();
        ValRep getZeroExtend(int targetSize);
        ValRep shink        (int targetSize);
        /** check define size*/
        [[nodiscard]] inline bool checkEqualBit(const ValRep& rhs) const{ return _len == rhs._len;}
        /** operation core*/
        ValRep bwOperator     (const ValRep& rhs,
                               const std::function<ull(ull, ull)>& operation);
        ValRep logicalOperator(const ValRep& rhs,
                               const std::function<bool(bool, bool)>& operation) const;
        ValRep cmpOperator    (ValRep& rhs,
                               const std::function<bool(ull* a, ull* b, int size)>& operation);
        ValRep eqOperator     (const ValRep& rhs, bool checkEq);
        /** get logical value in single bit*/
        bool   getLogicalValue() const;
        /** update value from slice*/
        ValRep slice(Slice sl);

        void update(ValRep& srcVal);
        void updateOnSlice(ValRep srcVal, Slice desSl);

        /** bit level control*/
        ull  getZeroMask(int startIdx, int stopIdx) const; //// mask zero at [startIdx, stopIdx) leave other bits with 1 value
        void fillZeroToValrep(int startBit, int stopBit); //// fill all bit in valrep bit absolute start bit
        void fillZeroToValrep(int startBit);

        /** convert to binary string*/
        std::string getBiStr();

        ValRep& operator = (const ValRep& rhs);

        //////// required equal bit operator
        ValRep operator &  (const ValRep& rhs);
        ValRep operator |  (const ValRep& rhs);
        ValRep operator ^  (const ValRep& rhs);
        ValRep operator == (const ValRep& rhs);
        ValRep operator != (const ValRep& rhs);
        //////// only one operand
        ValRep operator ~ ();

        //////// not required equal bit operator
        ////////////// but need zero extend
        ValRep operator && (const ValRep& rhs) const;
        ValRep operator || (const ValRep& rhs) const;
        ValRep operator !  () const;
        ValRep operator <  (ValRep& rhs);
        ValRep operator <= (ValRep& rhs);
        ValRep operator >  (ValRep& rhs);
        ValRep operator >= (ValRep& rhs);

        ///// not required equal bit operator
        ValRep operator +  (ValRep rhs);
        ValRep operator -  (ValRep rhs);
        ValRep operator *  (const ValRep&    ){assert(false);};
        ValRep operator /  (const ValRep&    ){assert(false);};
        ValRep operator %  (const ValRep&    ){assert(false);};
        ValRep operator << (const ValRep& rhs);
        ValRep operator << (int rhs);
        ValRep operator >> (const ValRep& rhs);
        ValRep operator >> (int rhs);


    };

}

#endif //KATHRYN_VALREP_H
