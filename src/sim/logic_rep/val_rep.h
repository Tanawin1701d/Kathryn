//
// Created by tanawin on 13/12/2566.
//

#ifndef KATHRYN_VALREP_H
#define KATHRYN_VALREP_H

#include "cassert"
#include "functional"
#include "string"
#include "cstdio"
#include "bitset"
#include "limits"
#include "iostream"

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

    constexpr int bit_size_of_ull = sizeof(ull) << 3;
    constexpr ull MAX_VALREP_RAW = ((ull)-1);

    class ValRepBase{
    public:
        int   _byteSize     = -1;
        int   _length       = -1;
        int   _continLength = -1; //// for more than 64 bit, it is checker for arrsize that can iterate from _val
        void* _val          = nullptr; //////// value at that idx

    public:
        ValRepBase(const int byte_size, void* val):
        _byteSize(byte_size),
        _val(val){}

        ValRepBase(){}

        /**
         *
         *  set value
         *
         ***/

        void set_var(ull x) const{
            if (_byteSize == 1){
                (*static_cast<uint8_t*>(_val)) = (uint8_t)(x);
            }else if (_byteSize == 2){
                (*static_cast<uint16_t*>(_val)) = (uint16_t)(x);
            }else if (_byteSize == 4){
                (*static_cast<uint32_t*>(_val)) = (uint32_t)(x);
            }else if (_byteSize == 8){
                if (_continLength == -1){
                    (*static_cast<uint64_t*>(_val)) = (uint64_t)(x);
                }else if (_continLength > 0){
                    (*static_cast<uint64_t*>(_val)) = (uint64_t)(x);
                    for(int i = 1; i < _continLength; i++){
                        *(static_cast<uint64_t*>(_val) + i) = (uint64_t)(0);
                    }
                }else{
                    assert(false);
                }
            }
        }

        void set_var_arr(const ull input_value, ull size){

            if (_continLength > 0){
                for (int write_iter = 0; write_iter < size; write_iter++){
                    set_var(input_value);
                }
            }else{
                if (_byteSize == 1){
                    std::fill(
                        static_cast<uint8_t*>(_val),
                        static_cast<uint8_t*>(_val) + size,
                        static_cast<uint8_t>(input_value));
                }else if (_byteSize == 2){
                    std::fill(
                        static_cast<uint16_t*>(_val),
                        static_cast<uint16_t*>(_val) + size,
                        static_cast<uint16_t>(input_value));
                }else if (_byteSize == 4){
                    std::fill(
                        static_cast<uint32_t*>(_val),
                        static_cast<uint32_t*>(_val) + size,
                        static_cast<uint32_t>(input_value));

                }else if (_byteSize == 8){
                    std::fill(
                        static_cast<uint64_t*>(_val),
                        static_cast<uint64_t*>(_val) + size,
                        static_cast<uint64_t>(input_value));

                }
            }
        }

        void set_large_var(std::vector<ull> x) const{
            assert(_continLength  > 0);
            assert(_byteSize     == 8);
            if (_continLength > 0){
                for(int i = 0; i < _continLength; i++){
                    *(static_cast<uint64_t*>(_val) + i) = x[i];
                }
            }
        }

        void set_var(const ValRepBase& rhs) const{
            ////// we must make sure that current chunk is bigger
            assert(_byteSize >= rhs._byteSize);
            assert(check_bg_or_eq_with_rhs(rhs));
            if (rhs._continLength == -1){
                set_var(rhs.get_val());
            }else{
                set_large_var(rhs.get_large_val());
            }

        }

        void set_size(int size){
            assert(size > 0);
            _length = size;
        }

        void set_contin_length(int size){
            assert(size > 0);
            _continLength = size;
        }

        /***
         *
         * check value
         *
         */

        bool check_bg_or_eq_with_rhs(const ValRepBase& rhs) const{
            assert(is_valid());
            assert(rhs.is_valid());
            if (_byteSize     < rhs._byteSize)    {return false;}
            if (_continLength < rhs._continLength){return false;}
            if (_length       < rhs._length )     {return false;}
            return true;

        }

        bool is_valid() const{
            return (_byteSize != -1) && (_length != -1);
        }

        bool is_in_used() const{
            return _val != nullptr;
        }

        /**
         *
         * get value
         */
        ull get_val()const{

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
                if (_continLength >= 0){
                    //// warning system should be declared here.
                }
                return *static_cast<uint64_t*>(_val);
            }
            assert(false);
        }

        std::vector<ull> get_large_val()const{
            assert(_continLength > 0);
            assert(_byteSize == 8);
            std::vector<ull> result;
            for (int idx = 0; idx < _continLength; idx++){
                result.push_back(*(static_cast<uint64_t*>(_val) + idx));
            }
            return result;
        }

        explicit operator ull() const{
            return get_val();
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
                if (_continLength == -1){
                    return {_byteSize, static_cast<uint64_t*>(_val) + idx};
                }
                if (_continLength > 0){
                    return {_byteSize, static_cast<uint64_t*>(_val) + (_continLength * idx)};
                }
            }
            assert(false);
        }

        ValRepBase copy(){
            ValRepBase cpy = *this;
            cpy._val = new uint8_t[_byteSize * std::max(1, _continLength)];
            std::copy((uint8_t*)_val,
                      (uint8_t*)_val + _byteSize *std::max(1, _continLength),
                      (uint8_t*)cpy._val);
            return cpy;
        }

        void fill_zero(int start_bit) const{

            switch (_byteSize){
            case 1 :{
                uint8_t max_bit = (1 << start_bit) - 1;
                (*static_cast<uint8_t*> (_val)) &= max_bit; break;
            }
            case 2 :{
                uint16_t max_bit = (1 << start_bit) - 1;
                (*static_cast<uint16_t*> (_val)) &= max_bit; break;
            }
            case 4 :{
                uint32_t max_bit = (1 << start_bit) - 1;
                (*static_cast<uint32_t*> (_val)) &= max_bit; break;
            }
            case 8 :{
                if (_continLength < 0){
                    uint64_t max_bit = (((ull)1) << start_bit) - 1;
                    (*static_cast<uint64_t*> (_val)) &= max_bit;
                    break;
                }
                assert(_continLength != 0);

                int start_fill_zero_idx        = start_bit / bit_size_of_ull;
                int start_partial_fill_zero_idx = start_bit % bit_size_of_ull;

                uint64_t* dayta = static_cast<uint64_t*>(_val);
                dayta[start_fill_zero_idx] &= (((ull)1) << start_partial_fill_zero_idx) - 1;

                for (int i = start_fill_zero_idx+1; i < _continLength; i++){
                    dayta[i] = 0;
                }
                break;
            }
            default: break;
            }

        }


        ValRepBase operator >> (int amt){

            ValRepBase new_val_rep = this->copy();

            switch (_byteSize){
                case 1 : {*static_cast<uint8_t* >(new_val_rep._val) >> amt; break;}
                case 2 : {*static_cast<uint16_t*>(new_val_rep._val) >> amt; break;}
                case 4 : {*static_cast<uint32_t*>(new_val_rep._val) >> amt; break;}
                case 8 :{
                    if (_continLength < 0){
                        *static_cast<uint64_t*>(new_val_rep._val) >> amt; break;
                    }
                    assert(_continLength != 0);

                    int full_shift    = amt / bit_size_of_ull;
                    int partial_shift = amt % bit_size_of_ull;

                    uint64_t* dayta = static_cast<uint64_t*>(new_val_rep._val);

                    for (int i = 0; i < _continLength; i++){
                        uint64_t new_val = 0;
                        /**
                         * full_shift = 1; partial_shift = 3
                         * |   d[2]   |     d[1]     |  d[0] = (d[2] << 3)(d[1]>>3)
                         */
                        //////// lower bit shift
                        if ((i + full_shift) < _continLength)
                            new_val =  dayta[i+full_shift] >> partial_shift;
                        //////// higher bit shift
                        if ((partial_shift != 0) && ((i + full_shift + 1) < _continLength))
                            new_val |= dayta[i+full_shift+1] << partial_shift;
                        dayta[i] = new_val;
                    }
                    break;
                }
                default: break;
            }
            return new_val_rep;
        }

        ValRepBase slice(const int start,const int stop){

            if (start == 0 && stop == _length){ return *this; }

            assert(start >= 0  && start < stop && stop <= _length );
            ValRepBase shifted = (*this) >> start;
            shifted.fill_zero(stop-start);
            return shifted;
        }

    };


    struct UintX_Base{

        virtual ull*get_data_base_ptr() = 0;

    };


    template<int arr_size>
    struct UintX: UintX_Base{
    public:
        ull _data[arr_size] = {};

        UintX(){}

        UintX(ull x){
            _data[0] = x;
        }

        UintX<arr_size> do_idx_by_idx (const UintX<arr_size>& rhs, const std::function<ull(ull, ull)>& op) const{
            UintX<arr_size> res;
            for (int i = 0; i < arr_size; i++){
                res._data[i] = op(_data[i],rhs._data[i]);
            }
            return res;
        }

        ull*get_data_base_ptr() override{
            return _data;
        }


        bool get_bi_value(){
            for (ull x : _data){
                if (x){
                    return true;
                }
            }
            return false;
        }

        std::string to_bi_str(){
            std::string pre_ret;
            for(int i = arr_size - 1; i >= 0; i--){
                std::bitset<bit_size_of_ull> binary_representation(_data[i]);
                pre_ret += binary_representation.to_string();
            }
            return pre_ret;
        }

        UintX<arr_size> operator & (const UintX<arr_size>& rhs) const{
            return do_idx_by_idx(rhs, [&](ull a, ull b){return a & b;});
        }
        UintX<arr_size> operator | (const UintX<arr_size>& rhs) const{
            return do_idx_by_idx(rhs, [&](ull a, ull b){return a | b;});
        }
        UintX<arr_size> operator ^ (const UintX<arr_size>& rhs) const{
            return do_idx_by_idx(rhs, [&](ull a, ull b){return a ^ b;});
        }
        UintX<arr_size> operator ~ () const{
            UintX<arr_size> res;
            for (int i = 0; i < arr_size; i++){
                res._data[i] = ~_data[i];
            }
            return res;
        }

        template<int T>
        UintX<arr_size> operator << (UintX<T> amt){
            return operator << (amt._data[0]);
        }

        UintX<arr_size> operator << (int amt){

            int full_shift    = amt / bit_size_of_ull;
            int partial_shift = amt % bit_size_of_ull;
            UintX<arr_size> result;

            for (int i = 0; i < arr_size; i++){
                ull lower_plate = _data[i] << partial_shift;

                ull higher_plate = (partial_shift == 0) ? 0ULL
                                                      : _data[i] >> (bit_size_of_ull-partial_shift);

                int des_lower_plate_idx  = i + full_shift;
                int des_higher_plate_idx = des_lower_plate_idx + 1;

                if (des_lower_plate_idx < arr_size){
                    result._data[des_lower_plate_idx] |= lower_plate;
                }
                if (des_higher_plate_idx < arr_size){
                    result._data[des_higher_plate_idx] |= higher_plate;
                }
            }
            return result;

        }

        template<int T>
        UintX<arr_size> operator >> (UintX<T> amt){
            return operator >> (amt._data[0]);
        }

        UintX<arr_size> operator >> (int amt){

            int full_shift = amt / bit_size_of_ull;
            int partial_shift = amt % bit_size_of_ull;
            UintX<arr_size> result;

            for (int i = 0; i < arr_size; i++){
                ull lower_plate = (partial_shift == 0) ? 0ULL
                                                     : (_data[i] << (bit_size_of_ull-partial_shift));

                ull higher_plate = _data[i] >> partial_shift;

                int des_lower_plate_idx  = i - full_shift - 1;
                int des_higher_plate_idx = des_lower_plate_idx + 1;

                if ((des_lower_plate_idx < arr_size) && (des_lower_plate_idx >= 0)){
                    result._data[des_lower_plate_idx] |= lower_plate;
                }
                if ((des_higher_plate_idx < arr_size) && (des_higher_plate_idx >= 0)){
                    result._data[des_higher_plate_idx] |= higher_plate;
                }
            }
            return result;

        }

        uint8_t operator && (const UintX<arr_size>& rhs){
            return get_bi_value() && rhs.get_bi_value();
        }
        uint8_t operator || (const UintX<arr_size>& rhs){
            return get_bi_value() || rhs.get_bi_value();
        }
        uint8_t operator ! (){
            return !get_bi_value();
        }

        uint8_t operator == (const UintX<arr_size>& rhs) const{

            for (int i = 0; i < arr_size; i++){
                if (_data[i] != rhs._data[i]){
                    return 0;
                }
            }
            return 1;
        }

        uint8_t operator != (const UintX<arr_size>& rhs) const{
            for (int i = 0; i < arr_size; i++){
                if (_data[i] != rhs._data[i]){
                    return 1;
                }
            }
            return 0;
        }

        uint8_t operator < (const UintX<arr_size>& rhs) const{
            for (int i = 0; i < arr_size; i++){
                if (_data[i] < rhs._data[i]){
                    return 1;
                }
                if (_data[i] > rhs._data[i]){
                    return 0;
                }
            }
            return 0;
        }

        uint8_t operator <= (const UintX<arr_size>& rhs) const{
            for (int i = 0; i < arr_size; i++){
                if (_data[i] < rhs._data[i]){
                    return 1;
                }
                if (_data[i] > rhs._data[i]){
                    return 0;
                }
            }
            return 1;
        }

        uint8_t operator > (const UintX<arr_size>& rhs) const{
            for (int i = 0; i < arr_size; i++){
                if (_data[i] > rhs._data[i]){
                    return 1;
                }
                if (_data[i] < rhs._data[i]){
                    return 0;
                }
            }
            return 0;
        }

        uint8_t operator >= (const UintX<arr_size>& rhs) const{
            for (int i = 0; i < arr_size; i++){
                if (_data[i] > rhs._data[i]){
                    return 1;
                }
                if (_data[i] < rhs._data[i]){
                    return 0;
                }
            }
            return 1;
        }

        UintX<arr_size> operator + (const UintX<arr_size>& rhs) const{
            UintX<arr_size> result;
            bool overflow = false;
            for (int i = 0; i < arr_size; i++){
                result._data[i] = _data[i] + rhs._data[i] + overflow;
                overflow = false;
                if ((result._data[i] < _data[i]) || (result._data[i] < rhs._data[i])){
                    overflow = true;
                }
            }
            return result;
        }

        UintX<arr_size> operator - (const UintX<arr_size>& rhs) const{
            assert(false);
        }

        UintX<arr_size> operator * (const UintX<arr_size>& rhs) const{
            assert(false);
        }

        UintX<arr_size> operator / (const UintX<arr_size>& rhs) const{
            assert(false);
        }

        UintX<arr_size> operator % (const UintX<arr_size>& rhs) const{
            assert(false);
        }

        UintX<arr_size>& operator = (const ull& eq) const{
            _data[0] = eq;
            return *this;
        }
        ////////                       96
        UintX<arr_size> build_mask(int size, int start){
            int start_idx = size / bit_size_of_ull;
            int partial_idx = size % bit_size_of_ull;

            UintX result = *this;

            ull start_cleaner = (1ULL << partial_idx) - 1;
            if (start_idx < arr_size){
                result._data[start_idx] = start_cleaner;
            }
            for (int idx = start_idx - 1; idx >= 0; idx--){
                result._data[idx] = UINT64_MAX;
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
            for (int i = 0; i < std::min(sz, arr_size); i++){
                result._data[i] = _data[i];
            }
        }

    };


    // int ma(){
    //     ValR<uint8_t> x;
    //     auto y = (x << 3ULL).fix_size(8);
    // }


    //
    // template<int _len>
    // class ValRep: public ValRepBase{
    // private:
    //     static const ull MASK_USED = (_len == bit_size_of_ull) ? MAX_VALREP_RAW : ( (((ull)1) << _len) -1 );
    //
    // public:
    //     ValRep(): ValRepBase(_len, 0){};
    //     ValRep(const ull value): ValRepBase(_len, value){}
    //
    //     template<int sl_start, int sl_stop>
    //     inline ull build_mask() const{
    //         int size = sl_stop - sl_start;
    //         ull mask = (size == bit_size_of_ull) ? -1 : ((((ull)1) << size) - 1);
    //         return mask;
    //     }
    //
    //     template<int sl_start, int sl_stop, int src_start>
    //     inline  ValRep<sl_stop - sl_start> slice_and_shift() const {
    //         assert((sl_start>=0) && (sl_start < sl_stop) && (sl_stop <= bit_size_of_ull));
    //         assert(src_start < bit_size_of_ull);
    //         ull mask = build_mask<sl_start, sl_stop>();
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
    //     void update_on_slice(const ValRep<sl_stop - sl_start>& rhs){
    //         ull mask = build_mask<sl_start, sl_stop>();
    //         mask = mask << sl_start;
    //         _val = _val & (~mask); ///////////// to clear old value
    //         _val = _val | rhs._val;
    //     }
    //
    //     template<int sl_start, int sl_stop>
    //     void update_on_slice(const ull rhs){
    //         ull mask = build_mask<sl_start, sl_stop>();
    //         mask = mask << sl_start;
    //         _val = _val & (~mask); ///////////// to clear old value
    //         _val = _val | rhs;
    //     }
    //
    //
    //
    //     template<int sl_start, int sl_stop, int fix_size>
    //     inline  ValRep<fix_size> slice() const {
    //         assert(fix_size >= (sl_stop - sl_start));
    //         assert((sl_start>=0) && (sl_start < sl_stop) && (sl_stop <= bit_size_of_ull));
    //         ull mask = build_mask<sl_start, sl_stop>();
    //         return ValRep<fix_size>(mask & (_val >> sl_start));
    //     }
    //
    //     template<int sl_start, int sl_stop>
    //     inline  ValRep<sl_stop - sl_start> slice() const {
    //         return slice<sl_start, sl_stop, sl_stop - sl_start>();
    //     }
    //
    //     bool get_logic_value(){
    //         return _val > 0;
    //     }
    //
    //     template<int des_size>
    //     inline ValRep<des_size> extend() const{
    //         ull des_val = 0;
    //         if (_val & 1){
    //             des_val = (des_size == bit_size_of_ull) ? -1 : ((1 << des_size)-1);
    //         }
    //         return ValRep<des_size>(des_val);
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
