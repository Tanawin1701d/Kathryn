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

    constexpr int bitSizeOfUll = sizeof(ull) << 3;

    class ValRepBase{
    protected:
        const int _length = -1;
        ull _val    = -1;
    public:
        ValRepBase(const int length, ull val): _length(length), _val(val){};

        std::string getBiStr(){
            std::string preRet ;
            std::bitset<bitSizeOfUll> binaryRepresentation(_val);
            preRet += binaryRepresentation.to_string();
            return preRet;
        }

        ull getVal()const {return _val;}
    };

    template<int _len>
    class ValRep: public ValRepBase{
    private:
        const ull MASK_USED = (_len == bitSizeOfUll) ? -1 : ((1 << _len) -1);

    public:
        ValRep(const ull value): ValRepBase(_len, value){}

        template<int sl_start, int sl_stop>
        inline ull buildMask(){
            int size = sl_stop - sl_start;
            ull mask = (size == bitSizeOfUll) ? -1 : ((1 << size) - 1);
            return mask;
        }

        template<int sl_start, int sl_stop, int src_start>
        inline ValRep<sl_stop - sl_start> sliceAndShift() const {
            assert((sl_start>=0) && (sl_start < sl_stop) && (sl_stop <= bitSizeOfUll));
            assert(src_start < bitSizeOfUll);
            ull mask = buildMask<sl_start, sl_stop>();
                mask = mask << sl_start;
                mask &= _val;
            if (sl_start <= src_start){
                mask = mask << (src_start - sl_start);
            }else{
                mask = mask >>  (sl_start - src_start);
            }
            return ValRep<sl_stop - sl_start>(mask);
        }

        template<int sl_start, int sl_stop>
        inline ValRep<sl_stop - sl_start> slice() const {
            assert((sl_start>=0) && (sl_start < sl_stop) && (sl_stop <= bitSizeOfUll));
            ull mask = buildMask<sl_start, sl_stop>();
            return ValRep<sl_stop - sl_start>(mask & (_val >> sl_start));
        }

        template<int sl_start, int sl_stop, int fixSize>
        inline ValRep<fixSize> slice() const {
            assert(fixSize >= (sl_stop - sl_start));
            assert((sl_start>=0) && (sl_start < sl_stop) && (sl_stop <= bitSizeOfUll));
            ull mask = buildMask<sl_start, sl_stop>();
            return ValRep<fixSize>(mask & (_val >> sl_start));
        }

        template<int sl_start, int sl_stop>
        void updateOnSlice(ValRep<sl_stop - sl_start>& rhs){
            ull mask = buildMask<sl_start, sl_stop>();
            mask = mask << sl_start;
            _val = _val & mask;
            _val = _val | rhs._val;
        }

        explicit operator bool(){
            return _val > 0;
        }

        bool getLogicValue(){
            return _val > 0;
        }


        inline ValRep& operator = (const ull value){ _val = value; return *this;}

        //////// required equal bit operator
        inline ValRep<_len> operator &  (const ValRep<_len>& rhs) const { return ValRep<_len>(_val  & rhs._val);}
        inline ValRep<_len> operator |  (const ValRep<_len>& rhs) const { return ValRep<_len>(_val  | rhs._val);}
        inline ValRep<_len> operator ^  (const ValRep<_len>& rhs) const { return ValRep<_len>(_val  ^ rhs._val);}
        inline ValRep<_len> operator == (const ValRep<_len>& rhs) const { return ValRep<_len>(_val == rhs._val);}
        inline ValRep<_len> operator != (const ValRep<_len>& rhs) const { return ValRep<_len>(_val != rhs._val);}
        //////// only one operand
        inline ValRep<_len> operator ~ () const { return ValRep<_len>((~_val) & MASK_USED);}

        //////// not required equal bit operator
        ////////////// but need zero extend
        inline ValRep<1> operator && (const ValRep<_len>& rhs) const{return ValRep<1>(_val && rhs._val);}
        inline ValRep<1> operator || (const ValRep<_len>& rhs) const{return ValRep<1>(_val || rhs._val);}
        inline ValRep<1> operator !  ()                        const{return ValRep<1>(!_val);           }
        inline ValRep<1> operator <  (const ValRep<_len>& rhs) const{return ValRep<1>(_val < rhs._val); }
        inline ValRep<1> operator <= (const ValRep<_len>& rhs) const{return ValRep<1>(_val <= rhs._val);}
        inline ValRep<1> operator >  (const ValRep<_len>& rhs) const{return ValRep<1>(_val > rhs._val); }
        inline ValRep<1> operator >= (const ValRep<_len>& rhs) const{return ValRep<1>(_val >= rhs._val);}

        ///// not required equal bit operator
        inline ValRep<_len> operator +  (const ValRep<_len>& rhs) const{return ValRep<_len>((_val + rhs._val) & MASK_USED);}
        inline ValRep<_len> operator -  (const ValRep<_len>& rhs) const{return ValRep<_len>((_val + ~(rhs) + ValRep<_len>(1))._val & MASK_USED);}
        inline ValRep<_len> operator *  (const ValRep<_len>&    ) const{assert(false);}
        inline ValRep<_len> operator /  (const ValRep<_len>&    ) const{assert(false);}
        inline ValRep<_len> operator %  (const ValRep<_len>&    ) const{assert(false);}

        inline ValRep<_len> operator << (const ValRep<_len>& rhs) const{return ValRep<_len>((_val << rhs._val)& MASK_USED);}
        inline ValRep<_len> operator >> (const ValRep<_len>& rhs) const{return ValRep<_len>((_val >> rhs._val)& MASK_USED);}

    };

}

#endif //KATHRYN_VALREP_H
