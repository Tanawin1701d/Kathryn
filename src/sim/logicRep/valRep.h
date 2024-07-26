//
// Created by tanawin on 13/12/2566.
//

#ifndef KATHRYN_VALREP_H
#define KATHRYN_VALREP_H

#include <cassert>
#include <functional>
#include <string>
#include <cstdio>
#include <bitset>
#include <limits>
#include <iostream>

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
    constexpr ull MAX_VALREP_RAW = ((ull)-1);

    class ValRepBase{
    public:
        int   _byteSize = -1;
        int   _length   = -1;
        void* _val      = nullptr; //////// value at that idx

    public:
        ValRepBase(const int byteSize, void* val):
        _byteSize(byteSize),
        _val(val){};

        ValRepBase(){}

        void setVar(ull x) const{
            if (_byteSize == 1){
                (*static_cast<uint8_t*>(_val)) = (uint8_t)(x);
            }else if (_byteSize == 2){
                (*static_cast<uint16_t*>(_val)) = (uint16_t)(x);
            }else if (_byteSize == 4){
                (*static_cast<uint32_t*>(_val)) = (uint32_t)(x);
            }else if (_byteSize == 8){
                (*static_cast<uint64_t*>(_val)) = (uint64_t)(x);
            }
        }

        bool isInUsed() const{
            return _val != nullptr;
        }

        void setSize(int size){
            _length = size;
        }

        ull getVal()const{

            if (_byteSize == 1){
                return *static_cast<uint8_t*>(_val);
            }
            if (_byteSize == 2){
                return *static_cast<uint16_t*>(_val);
            }
            if (_byteSize == 4){
                return *static_cast<uint32_t*>(_val);
            }
            if (_byteSize == 8){
                return *static_cast<uint64_t*>(_val);
            }
            assert(false);


        }

        explicit operator ull() const{
            return getVal();
        }

        ValRepBase& operator = (const ValRepBase& rhs) = default;

        ValRepBase operator [] (ull idx){
            if (_byteSize == 1){
                return {_byteSize, static_cast<uint8_t*>(_val) + idx};
            }
            if (_byteSize == 2){
                return {_byteSize, static_cast<uint16_t*>(_val) + idx};
            }
            if (_byteSize == 4){
                return {_byteSize, static_cast<uint32_t*>(_val) + idx};
            }
            if (_byteSize == 8){
                return {_byteSize, static_cast<uint64_t*>(_val) + idx};
            }
            assert(false);
        }


    };


    template<int arrSize>
    struct UintX{
    public:
        ull _data[arrSize] = {};

        UintX(){}

        UintX(ull x){
            _data[0] = x;
        }

        UintX<arrSize> doIdxByIdx (const UintX<arrSize>& rhs, const std::function<ull(ull, ull)>& op) const{
            UintX<arrSize> res;
            for (int i = 0; i < arrSize; i++){
                res._data[i] = op(_data[i],rhs._data[i]);
            }
            return res;
        }

        bool getBiValue(){
            for (ull x : _data){
                if (x){
                    return true;
                }
            }
            return false;
        }

        UintX<arrSize> operator & (const UintX<arrSize>& rhs) const{
            return doIdxByIdx(rhs, [&](ull a, ull b){return a & b;});
        }
        UintX<arrSize> operator | (const UintX<arrSize>& rhs) const{
            return doIdxByIdx(rhs, [&](ull a, ull b){return a | b;});
        }
        UintX<arrSize> operator ^ (const UintX<arrSize>& rhs) const{
            return doIdxByIdx(rhs, [&](ull a, ull b){return a ^ b;});
        }
        UintX<arrSize> operator ~ () const{
            UintX<arrSize> res;
            for (int i = 0; i < arrSize; i++){
                res._data[i] = ~_data[i];
            }
            return res;
        }

        UintX<arrSize> operator << (int amt){

            int fullShift = amt / bitSizeOfUll;
            int partialShift = amt % bitSizeOfUll;
            UintX<arrSize> result;

            for (int i = 0; i < arrSize; i++){
                ull lowerPlate = _data[i] << partialShift;

                ull higherPlate = (partialShift == 0) ? 0ULL
                                                      : _data[i] >> (bitSizeOfUll-partialShift);

                int desLowerPlateIdx  = i + fullShift;
                int desHigherPlateIdx = desLowerPlateIdx + 1;

                if (desLowerPlateIdx < arrSize){
                    _data[desLowerPlateIdx] |= lowerPlate;
                }
                if (desHigherPlateIdx < arrSize){
                    _data[desHigherPlateIdx] |= higherPlate;
                }
            }
            return result;

        }

        UintX<arrSize> operator >> (int amt){

            int fullShift = amt / bitSizeOfUll;
            int partialShift = amt % bitSizeOfUll;
            UintX<arrSize> result;

            for (int i = 0; i < arrSize; i++){
                ull lowerPlate = (partialShift == 0) ? 0ULL
                                                     : (_data[i] << (bitSizeOfUll-partialShift));

                ull higherPlate = _data[i] >> partialShift;

                int desLowerPlateIdx  = i - fullShift - 1;
                int desHigherPlateIdx = desLowerPlateIdx + 1;

                if ((desLowerPlateIdx < arrSize) && (desLowerPlateIdx >= 0)){
                    _data[desLowerPlateIdx] |= lowerPlate;
                }
                if ((desHigherPlateIdx < arrSize) && (desHigherPlateIdx >= 0)){
                    _data[desHigherPlateIdx] |= higherPlate;
                }
            }
            return result;

        }

        uint8_t operator && (const UintX<arrSize>& rhs){
            return getBiValue() && rhs.getBiValue();
        }
        uint8_t operator || (const UintX<arrSize>& rhs){
            return getBiValue() || rhs.getBiValue();
        }
        uint8_t operator ! (){
            return !getBiValue();
        }

        uint8_t operator == (const UintX<arrSize>& rhs) const{

            for (int i = 0; i < arrSize; i++){
                if (_data[i] != rhs._data[i]){
                    return 0;
                }
            }
            return 1;
        }

        uint8_t operator != (const UintX<arrSize>& rhs) const{
            for (int i = 0; i < arrSize; i++){
                if (_data[i] != rhs._data[i]){
                    return 1;
                }
            }
            return 0;
        }

        uint8_t operator < (const UintX<arrSize>& rhs) const{
            for (int i = 0; i < arrSize; i++){
                if (_data[i] < rhs._data[i]){
                    return 1;
                }
                if (_data[i] > rhs._data[i]){
                    return 0;
                }
            }
            return 0;
        }

        uint8_t operator <= (const UintX<arrSize>& rhs) const{
            for (int i = 0; i < arrSize; i++){
                if (_data[i] < rhs._data[i]){
                    return 1;
                }
                if (_data[i] > rhs._data[i]){
                    return 0;
                }
            }
            return 1;
        }

        uint8_t operator > (const UintX<arrSize>& rhs) const{
            for (int i = 0; i < arrSize; i++){
                if (_data[i] > rhs._data[i]){
                    return 1;
                }
                if (_data[i] < rhs._data[i]){
                    return 0;
                }
            }
            return 0;
        }

        uint8_t operator >= (const UintX<arrSize>& rhs) const{
            for (int i = 0; i < arrSize; i++){
                if (_data[i] > rhs._data[i]){
                    return 1;
                }
                if (_data[i] < rhs._data[i]){
                    return 0;
                }
            }
            return 1;
        }

        UintX<arrSize> operator + (const UintX<arrSize>& rhs) const{
            UintX<arrSize> result;
            bool overflow = false;
            for (int i = 0; i < arrSize; i++){
                result._data[i] = _data[i] + rhs._data[i] + overflow;
                overflow = false;
                if ((result._data[i] < _data[i]) || (result._data[i] < rhs._data[i])){
                    overflow = true;
                }
            }
            return result;
        }

        UintX<arrSize> operator - (const UintX<arrSize>& rhs) const{
            assert(false);
        }

        UintX<arrSize> operator * (const UintX<arrSize>& rhs) const{
            assert(false);
        }

        UintX<arrSize> operator / (const UintX<arrSize>& rhs) const{
            assert(false);
        }

        UintX<arrSize> operator % (const UintX<arrSize>& rhs) const{
            assert(false);
        }

        UintX<arrSize>& operator = (const ull& eq) const{
            _data[0] = eq;
            return *this;
        }

        UintX<arrSize> buildMask(int size, int start){
            int startIdx = size / bitSizeOfUll;
            int partialIdx = size % bitSizeOfUll;

            UintX result = *this;

            ull startCleaner = (1ULL << partialIdx) - 1;
            result._data[startIdx] &= startCleaner;
            for (int idx = startIdx + 1; idx < arrSize; idx++){
                result._data[idx] = 0;
            }
            return result << start;
        }

        explicit operator uint8_t() const{
            return _data[0];
        }

        explicit operator uint16_t() const{
            return _data[0];
        }

        explicit operator uint32_t() const{
            return _data[0];
        }

        explicit operator uint64_t() const{
            return _data[0];
        }

        explicit operator bool() const{
            return _data[0];
        }

        template<int sz>
        explicit operator UintX<sz>() const{
            UintX<sz> result;
            for (int i = 0; i < std::min(sz, arrSize); i++){
                result._data[i] = _data[i];
            }
        }








    };


    // int ma(){
    //     ValR<uint8_t> x;
    //     auto y = (x << 3ULL).fixSize(8);
    // }


    //
    // template<int _len>
    // class ValRep: public ValRepBase{
    // private:
    //     static const ull MASK_USED = (_len == bitSizeOfUll) ? MAX_VALREP_RAW : ( (((ull)1) << _len) -1 );
    //
    // public:
    //     ValRep(): ValRepBase(_len, 0){};
    //     ValRep(const ull value): ValRepBase(_len, value){}
    //
    //     template<int sl_start, int sl_stop>
    //     inline ull buildMask() const{
    //         int size = sl_stop - sl_start;
    //         ull mask = (size == bitSizeOfUll) ? -1 : ((((ull)1) << size) - 1);
    //         return mask;
    //     }
    //
    //     template<int sl_start, int sl_stop, int src_start>
    //     inline  ValRep<sl_stop - sl_start> sliceAndShift() const {
    //         assert((sl_start>=0) && (sl_start < sl_stop) && (sl_stop <= bitSizeOfUll));
    //         assert(src_start < bitSizeOfUll);
    //         ull mask = buildMask<sl_start, sl_stop>();
    //             mask = mask << sl_start;
    //             mask &= _val;
    //         if (sl_start <= src_start){
    //             mask = mask << (src_start - sl_start);
    //         }else{
    //             mask = mask >>  (sl_start - src_start);
    //         }
    //         return ValRep<sl_stop - sl_start>(mask);
    //     }
    //
    //     template<int sl_start, int sl_stop>
    //     void updateOnSlice(const ValRep<sl_stop - sl_start>& rhs){
    //         ull mask = buildMask<sl_start, sl_stop>();
    //         mask = mask << sl_start;
    //         _val = _val & (~mask); ///////////// to clear old value
    //         _val = _val | rhs._val;
    //     }
    //
    //     template<int sl_start, int sl_stop>
    //     void updateOnSlice(const ull rhs){
    //         ull mask = buildMask<sl_start, sl_stop>();
    //         mask = mask << sl_start;
    //         _val = _val & (~mask); ///////////// to clear old value
    //         _val = _val | rhs;
    //     }
    //
    //
    //
    //     template<int sl_start, int sl_stop, int fixSize>
    //     inline  ValRep<fixSize> slice() const {
    //         assert(fixSize >= (sl_stop - sl_start));
    //         assert((sl_start>=0) && (sl_start < sl_stop) && (sl_stop <= bitSizeOfUll));
    //         ull mask = buildMask<sl_start, sl_stop>();
    //         return ValRep<fixSize>(mask & (_val >> sl_start));
    //     }
    //
    //     template<int sl_start, int sl_stop>
    //     inline  ValRep<sl_stop - sl_start> slice() const {
    //         return slice<sl_start, sl_stop, sl_stop - sl_start>();
    //     }
    //
    //     bool getLogicValue(){
    //         return _val > 0;
    //     }
    //
    //     template<int desSize>
    //     inline ValRep<desSize> extend() const{
    //         ull desVal = 0;
    //         if (_val & 1){
    //             desVal = (desSize == bitSizeOfUll) ? -1 : ((1 << desSize)-1);
    //         }
    //         return ValRep<desSize>(desVal);
    //     }
    //
    //
    //     inline ValRep& operator = (const ull value){ _val = value; return *this;}
    //     inline ValRep& operator = (const ValRep<_len>& value){_val = value._val; return *this;}
    //
    //     //////// required equal bit operator
    //     inline ValRep<_len> operator &  (const ValRep<_len>& rhs) const { return ValRep<_len>(_val  & rhs._val);}
    //     inline ValRep<_len> operator |  (const ValRep<_len>& rhs) const { return ValRep<_len>(_val  | rhs._val);}
    //     inline ValRep<_len> operator ^  (const ValRep<_len>& rhs) const { return ValRep<_len>(_val  ^ rhs._val);}
    //     inline ValRep<1>    operator == (const ValRep<_len>& rhs) const { return ValRep<1>(_val == rhs._val);}
    //     inline ValRep<1>    operator != (const ValRep<_len>& rhs) const { return ValRep<1>(_val != rhs._val);}
    //     //////// only one operand
    //     inline ValRep<_len> operator ~ () const { return ValRep<_len>((~_val) & MASK_USED);}
    //
    //     //////// not required equal bit operator
    //     ////////////// but need zero extend
    //     inline ValRep<1> operator && (const ValRep<_len>& rhs) const{return ValRep<1>(_val && rhs._val);}
    //     inline ValRep<1> operator || (const ValRep<_len>& rhs) const{return ValRep<1>(_val || rhs._val);}
    //     inline ValRep<1> operator !  ()                        const{return ValRep<1>(!_val);           }
    //     inline ValRep<1> operator <  (const ValRep<_len>& rhs) const{return ValRep<1>(_val < rhs._val); }
    //     inline ValRep<1> operator <= (const ValRep<_len>& rhs) const{return ValRep<1>(_val <= rhs._val);}
    //     inline ValRep<1> operator >  (const ValRep<_len>& rhs) const{return ValRep<1>(_val > rhs._val); }
    //     inline ValRep<1> operator >= (const ValRep<_len>& rhs) const{return ValRep<1>(_val >= rhs._val);}
    //
    //     ///// not required equal bit operator
    //     inline ValRep<_len> operator +  (const ValRep<_len>& rhs) const{return ValRep<_len>((_val + rhs._val) & MASK_USED);}
    //     inline ValRep<_len> operator -  (const ValRep<_len>& rhs) const{return ValRep<_len>(((*this) + ~(rhs) + ValRep<_len>(1))._val & MASK_USED);}
    //     inline ValRep<_len> operator *  (const ValRep<_len>&    ) const{assert(false);}
    //     inline ValRep<_len> operator /  (const ValRep<_len>&    ) const{assert(false);}
    //     inline ValRep<_len> operator %  (const ValRep<_len>&    ) const{assert(false);}
    //
    //     inline ValRep<_len> operator << (const ValRep<_len>& rhs) const{return ValRep<_len>((_val << rhs._val)& MASK_USED);}
    //     inline ValRep<_len> operator >> (const ValRep<_len>& rhs) const{return ValRep<_len>((_val >> rhs._val)& MASK_USED);}
    //
    //
    //
    //     inline explicit operator bool() const {return _val;}
    //
    // };

}

#endif //KATHRYN_VALREP_H
