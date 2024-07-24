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
        const int _byteSize = -1;
        int       _length   = -1;
        void*     _val      = nullptr; //////// value at that idx

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

        ValRepBase& operator = (const ValRepBase& rhs){
            assert(_length == rhs._length);
            assert(_byteSize == rhs._byteSize);
            setVar(rhs.getVal());
            return *this;
        }

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


    template<typename T>
    struct ValR{
        T _data;

        ValR(const T& data):_data(data){}
        ////////// bitwise
        __attribute__((always_inline)) ValR<T> operator &  (const ValR<T>& rhs) const{ return {(T)(_data & rhs._data)}; }

        __attribute__((always_inline)) ValR<T> operator |  (const ValR<T>& rhs) const{ return {(T)(_data | rhs._data)}; }
        __attribute__((always_inline)) ValR<T> operator ^  (const ValR<T>& rhs) const{ return {(T)(_data ^ rhs._data)}; }
        __attribute__((always_inline)) ValR<T> operator ~  ()                   const{ return {(T)(~_data)};            }
        template<typename R>
        __attribute__((always_inline)) ValR<T> operator << (const ValR<R>& rhs) const{ return {(T)(_data << rhs._data)};}
        template<typename R>
        __attribute__((always_inline)) ValR<T> operator >> (const ValR<R>& rhs) const{ return {(T)(_data >> rhs._data)};}
        ////////// logical
        __attribute__((always_inline)) ValR<uint8_t> operator && (const ValR<T>& rhs) const{ return {(uint8_t)(_data && rhs._data)};}
        __attribute__((always_inline)) ValR<uint8_t> operator || (const ValR<T>& rhs) const{ return {(uint8_t)(_data || rhs._data)};}
        __attribute__((always_inline)) ValR<uint8_t> operator !  ()                   const{ return {(uint8_t)(!_data)};}
        ////////// relational
        __attribute__((always_inline)) ValR<uint8_t> operator == (const ValR<T>& rhs) const{ return {(uint8_t)(_data == rhs._data)};}
        __attribute__((always_inline)) ValR<uint8_t> operator != (const ValR<T>& rhs) const{ return {(uint8_t)(_data != rhs._data)};}
        __attribute__((always_inline)) ValR<uint8_t> operator <  (const ValR<T>& rhs) const{ return {(uint8_t)(_data <  rhs._data)};}
        __attribute__((always_inline)) ValR<uint8_t> operator <= (const ValR<T>& rhs) const{ return {(uint8_t)(_data <= rhs._data)};}
        __attribute__((always_inline)) ValR<uint8_t> operator >  (const ValR<T>& rhs) const{ return {(uint8_t)(_data >  rhs._data)};}
        __attribute__((always_inline)) ValR<uint8_t> operator >= (const ValR<T>& rhs) const{ return {(uint8_t)(_data >= rhs._data)};}

        ////////// relational
        __attribute__((always_inline)) ValR<T> operator +  (const ValR<T>& rhs) const{ return {(T)(_data + rhs._data)};}
        __attribute__((always_inline)) ValR<T> operator -  (const ValR<T>& rhs) const{ return {(T)(_data - rhs._data)};}
        __attribute__((always_inline)) ValR<T> operator *  (const ValR<T>& rhs) const{ assert(false);}
        __attribute__((always_inline)) ValR<T> operator /  (const ValR<T>& rhs) const{ assert(false);}
        __attribute__((always_inline)) ValR<T> operator %  (const ValR<T>& rhs) const{ assert(false);}

        __attribute__((always_inline)) ValR<T>& operator &=  (const ValR<T>& rhs){
            _data &= rhs._data; return *this;
        }

        __attribute__((always_inline)) ValR<T>& operator |=  (const ValR<T>& rhs){
            _data |= rhs._data; return *this;
        }

        ////// bit extend
        __attribute__((always_inline)) void ext(int size, const ValR<uint8_t> x ){
            if (x){
                if (size == (sizeof(T)*8)){
                    _data = std::numeric_limits<T>::max();
                }
                _data = (((T)1 << size) - 1);
            }
            _data = 0;
        }

        __attribute__((always_inline)) T    buildMask (const int size){
            return ((size >= (sizeof(T)*8)) ? std::numeric_limits<T>::max() : (((T)1 << size) - 1));
        }

        ////// fundamental function do it own data
        __attribute__((always_inline)) ValR<T>& operator = (const ValR<T>& rhs){_data = rhs._data; return *this;}
        __attribute__((always_inline)) void    fixSize (int size){
            _data  &= buildMask(size);
        }

        __attribute__((always_inline)) void    clear (const int a, const int b){
            const int size = b-a;
            T mask = buildMask(size);
            _data &= (~(mask << a));
        }

        /////// do it on slice and shift

        __attribute__((always_inline)) ValR<T>  slice (const int a, const int b) const{
            uint8_t size = b-a;
            uint8_t mask = ((size >= (sizeof(T)*8)) ? std::numeric_limits<T>::max() : (((T)1 << size) - 1));
            return (T)((T)(_data >> a) & mask);
        }

        __attribute__((always_inline)) ValR<T>  shift(const int start) const{
            return ValR<T>((T)(_data << start));
        }


        __attribute__((always_inline)) operator bool() const{
            return _data;
        }

        template<typename D>
        __attribute__((always_inline)) ValR<D> cast() const{
            return ValR<D>((D)_data); //// if D is less than T, system mustbe terminate
        }


    };


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
