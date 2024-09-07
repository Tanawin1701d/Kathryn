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

#include "sim/debugger/simDebugger.h"

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
        int   _byteSize     = -1;
        int   _length       = -1;
        int   _continLength = -1; //// for more than 64 bit, it is checker for arrsize that can iterate from _val
        void* _val          = nullptr; //////// value at that idx

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
                if (_continLength == -1){
                    (*static_cast<uint64_t*>(_val)) = (uint64_t)(x);
                }else if (_continLength > 0){
                    (*static_cast<uint64_t*>(_val)) = (uint64_t)(x);
                    for(int i = 1; i < _continLength; i++){
                        *(static_cast<uint64_t*>(_val) + i) = (uint64_t)(0);
                    }
                }else{
                    smAssert(false, "valRep detect unknown range of this value");
                }
            }else{
                smAssert(false, "valRep detect unknown byte type of this value");
            }

        }

        void setLargeVar(std::vector<ull> x) const{
            assert(_continLength  > 0);
            assert(_byteSize     == 8);
            if (_continLength > 0){
                for(int i = 0; i < _continLength; i++){
                    *(static_cast<uint64_t*>(_val) + i) = x[i];
                }
            }
        }

        bool isInUsed() const{
            return _val != nullptr;
        }

        void setSize(int size){
            assert(size > 0);
            _length = size;
        }

        void setContinLength(int size){
            assert(size > 0);
            _continLength = size;
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
                if (_continLength >= 0){
                    smWarn(false, "this variable is larger than 8 byte integer");
                }
                return *static_cast<uint64_t*>(_val);
            }
            smAssert(false, "invalid byteSize for get var");


        }

        std::vector<ull> getLargeVal()const{

            smAssert(_continLength > 0, "invalid range for large var");
            smAssert(_byteSize == 8   , "invalid byteSize for large var");


            std::vector<ull> result;
            for (int idx = 0; idx < _continLength; idx++){
                result.push_back(*(static_cast<uint64_t*>(_val) + idx));
            }
            return result;
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
                if (_continLength == -1){
                    return {_byteSize, static_cast<uint64_t*>(_val) + idx};
                }
                if (_continLength > 0){
                    return {_byteSize, static_cast<uint64_t*>(_val) + (_continLength * idx)};
                }
            }
            smAssert(false, "invalid byte type while slicing");

        }

    };


    // struct UintX_Base{
    //
    //     virtual ull* getDataBase() = 0;
    //
    // };
    //
    //
    // template<int arrSize>
    // struct UintX: UintX_Base{
    // public:
    //     ull _data[arrSize] = {};
    //
    //     UintX(){}
    //
    //     UintX(ull x){
    //         _data[0] = x;
    //     }
    //
    //     UintX<arrSize> doIdxByIdx (const UintX<arrSize>& rhs, const std::function<ull(ull, ull)>& op) const{
    //         UintX<arrSize> res;
    //         for (int i = 0; i < arrSize; i++){
    //             res._data[i] = op(_data[i],rhs._data[i]);
    //         }
    //         return res;
    //     }
    //
    //     ull* getDataBase() override{
    //         return _data;
    //     }
    //
    //
    //     bool getBiValue(){
    //         for (ull x : _data){
    //             if (x){
    //                 return true;
    //             }
    //         }
    //         return false;
    //     }
    //
    //     std::string toBiStr(){
    //         std::string preRet;
    //         for(int i = arrSize - 1; i >= 0; i--){
    //             std::bitset<bitSizeOfUll> binaryRepresentation(_data[i]);
    //             preRet += binaryRepresentation.to_string();
    //         }
    //         return preRet;
    //     }
    //
    //     UintX<arrSize> operator & (const UintX<arrSize>& rhs) const{
    //         return doIdxByIdx(rhs, [&](ull a, ull b){return a & b;});
    //     }
    //     UintX<arrSize> operator | (const UintX<arrSize>& rhs) const{
    //         return doIdxByIdx(rhs, [&](ull a, ull b){return a | b;});
    //     }
    //     UintX<arrSize> operator ^ (const UintX<arrSize>& rhs) const{
    //         return doIdxByIdx(rhs, [&](ull a, ull b){return a ^ b;});
    //     }
    //     UintX<arrSize> operator ~ () const{
    //         UintX<arrSize> res;
    //         for (int i = 0; i < arrSize; i++){
    //             res._data[i] = ~_data[i];
    //         }
    //         return res;
    //     }
    //
    //     template<int T>
    //     UintX<arrSize> operator << (UintX<T> amt){
    //         return operator << (amt._data[0]);
    //     }
    //
    //     UintX<arrSize> operator << (int amt){
    //
    //         int fullShift    = amt / bitSizeOfUll;
    //         int partialShift = amt % bitSizeOfUll;
    //         UintX<arrSize> result;
    //
    //         for (int i = 0; i < arrSize; i++){
    //             ull lowerPlate = _data[i] << partialShift;
    //
    //             ull higherPlate = (partialShift == 0) ? 0ULL
    //                                                   : _data[i] >> (bitSizeOfUll-partialShift);
    //
    //             int desLowerPlateIdx  = i + fullShift;
    //             int desHigherPlateIdx = desLowerPlateIdx + 1;
    //
    //             if (desLowerPlateIdx < arrSize){
    //                 result._data[desLowerPlateIdx] |= lowerPlate;
    //             }
    //             if (desHigherPlateIdx < arrSize){
    //                 result._data[desHigherPlateIdx] |= higherPlate;
    //             }
    //         }
    //         return result;
    //
    //     }
    //
    //     template<int T>
    //     UintX<arrSize> operator >> (UintX<T> amt){
    //         return operator >> (amt._data[0]);
    //     }
    //
    //     UintX<arrSize> operator >> (int amt){
    //
    //         int fullShift = amt / bitSizeOfUll;
    //         int partialShift = amt % bitSizeOfUll;
    //         UintX<arrSize> result;
    //
    //         for (int i = 0; i < arrSize; i++){
    //             ull lowerPlate = (partialShift == 0) ? 0ULL
    //                                                  : (_data[i] << (bitSizeOfUll-partialShift));
    //
    //             ull higherPlate = _data[i] >> partialShift;
    //
    //             int desLowerPlateIdx  = i - fullShift - 1;
    //             int desHigherPlateIdx = desLowerPlateIdx + 1;
    //
    //             if ((desLowerPlateIdx < arrSize) && (desLowerPlateIdx >= 0)){
    //                 result._data[desLowerPlateIdx] |= lowerPlate;
    //             }
    //             if ((desHigherPlateIdx < arrSize) && (desHigherPlateIdx >= 0)){
    //                 result._data[desHigherPlateIdx] |= higherPlate;
    //             }
    //         }
    //         return result;
    //
    //     }
    //
    //     uint8_t operator && (const UintX<arrSize>& rhs){
    //         return getBiValue() && rhs.getBiValue();
    //     }
    //     uint8_t operator || (const UintX<arrSize>& rhs){
    //         return getBiValue() || rhs.getBiValue();
    //     }
    //     uint8_t operator ! (){
    //         return !getBiValue();
    //     }
    //
    //     uint8_t operator == (const UintX<arrSize>& rhs) const{
    //
    //         for (int i = 0; i < arrSize; i++){
    //             if (_data[i] != rhs._data[i]){
    //                 return 0;
    //             }
    //         }
    //         return 1;
    //     }
    //
    //     uint8_t operator != (const UintX<arrSize>& rhs) const{
    //         for (int i = 0; i < arrSize; i++){
    //             if (_data[i] != rhs._data[i]){
    //                 return 1;
    //             }
    //         }
    //         return 0;
    //     }
    //
    //     uint8_t operator < (const UintX<arrSize>& rhs) const{
    //         for (int i = 0; i < arrSize; i++){
    //             if (_data[i] < rhs._data[i]){
    //                 return 1;
    //             }
    //             if (_data[i] > rhs._data[i]){
    //                 return 0;
    //             }
    //         }
    //         return 0;
    //     }
    //
    //     uint8_t operator <= (const UintX<arrSize>& rhs) const{
    //         for (int i = 0; i < arrSize; i++){
    //             if (_data[i] < rhs._data[i]){
    //                 return 1;
    //             }
    //             if (_data[i] > rhs._data[i]){
    //                 return 0;
    //             }
    //         }
    //         return 1;
    //     }
    //
    //     uint8_t operator > (const UintX<arrSize>& rhs) const{
    //         for (int i = 0; i < arrSize; i++){
    //             if (_data[i] > rhs._data[i]){
    //                 return 1;
    //             }
    //             if (_data[i] < rhs._data[i]){
    //                 return 0;
    //             }
    //         }
    //         return 0;
    //     }
    //
    //     uint8_t operator >= (const UintX<arrSize>& rhs) const{
    //         for (int i = 0; i < arrSize; i++){
    //             if (_data[i] > rhs._data[i]){
    //                 return 1;
    //             }
    //             if (_data[i] < rhs._data[i]){
    //                 return 0;
    //             }
    //         }
    //         return 1;
    //     }
    //
    //     UintX<arrSize> operator + (const UintX<arrSize>& rhs) const{
    //         UintX<arrSize> result;
    //         bool overflow = false;
    //         for (int i = 0; i < arrSize; i++){
    //             result._data[i] = _data[i] + rhs._data[i] + overflow;
    //             overflow = false;
    //             if ((result._data[i] < _data[i]) || (result._data[i] < rhs._data[i])){
    //                 overflow = true;
    //             }
    //         }
    //         return result;
    //     }
    //
    //     UintX<arrSize> operator - (const UintX<arrSize>& rhs) const{
    //         assert(false);
    //     }
    //
    //     UintX<arrSize> operator * (const UintX<arrSize>& rhs) const{
    //         assert(false);
    //     }
    //
    //     UintX<arrSize> operator / (const UintX<arrSize>& rhs) const{
    //         assert(false);
    //     }
    //
    //     UintX<arrSize> operator % (const UintX<arrSize>& rhs) const{
    //         assert(false);
    //     }
    //
    //     UintX<arrSize>& operator = (const ull& eq) const{
    //         _data[0] = eq;
    //         return *this;
    //     }
    //     ////////                       96
    //     UintX<arrSize> buildMask(int size, int start){
    //         int startIdx = size / bitSizeOfUll;
    //         int partialIdx = size % bitSizeOfUll;
    //
    //         UintX result = *this;
    //
    //         ull startCleaner = (1ULL << partialIdx) - 1;
    //         if (startIdx < arrSize){
    //             result._data[startIdx] = startCleaner;
    //         }
    //         for (int idx = startIdx - 1; idx >= 0; idx--){
    //             result._data[idx] = UINT64_MAX;
    //         }
    //         return result << start;
    //     }
    //
    //     explicit operator uint8_t() const{
    //         return _data[0];
    //     }
    //
    //     explicit operator uint16_t() const{
    //         return _data[0];
    //     }
    //
    //     explicit operator uint32_t() const{
    //         return _data[0];
    //     }
    //
    //     explicit operator uint64_t() const{
    //         return _data[0];
    //     }
    //
    //     explicit operator bool() const{
    //         return _data[0];
    //     }
    //
    //     template<int sz>
    //     explicit operator UintX<sz>() const{
    //         UintX<sz> result;
    //         for (int i = 0; i < std::min(sz, arrSize); i++){
    //             result._data[i] = _data[i];
    //         }
    //     }
    //
    // };

}

#endif //KATHRYN_VALREP_H
