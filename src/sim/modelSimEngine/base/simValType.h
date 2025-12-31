//
// Created by tanawin on 23/7/2024.
//

#ifndef SIMVALTYPE_H
#define SIMVALTYPE_H

#include<string>
#include<cassert>
#include <utility>
#include <model/hwComponent/abstract/identifiable.h>
#include <model/hwComponent/abstract/Slice.h>
#include <util/numberic/numConvert.h>

namespace kathryn{


    class Operable;

    enum SIM_VALREP_TYPE{
        SVT_U8   = 0,
        SVT_U16  = 1,
        SVT_U32  = 2,
        SVT_U64  = 3,
        SVT_U64M = 4,
        SVT_ERR  = 5,
        SVT_CNT  = 6
    };

    enum SIM_VALREP_TYPE_INT{
        SVTI_8   = 0,
        SVTI_16  = 1,
        SVTI_32  = 2,
        SVTI_64  = 3,
        SVTI_64M = 4,
        SVTI_ERR  = 5,
        SVTI_CNT  = 6
    };

    struct SIM_VALREP_TYPE_ALL;
    struct SIM_VALREP_TYPE_I_ALL;
    SIM_VALREP_TYPE_ALL getMatchSVT_ALL  (Operable* opr1);
    SIM_VALREP_TYPE     getMatchSVT      (int size);
    SIM_VALREP_TYPE_INT getMatchSVTI     (int size);

    std::string         SVT_toUnitType   (SIM_VALREP_TYPE_ALL svt);
    std::string         SVTI_toUnitType  (SIM_VALREP_TYPE_I_ALL svti);

    std::string         SVT_toUnitRefType(SIM_VALREP_TYPE_ALL svt);
    std::string         SVT_toUnitPtrType(SIM_VALREP_TYPE_ALL svt);
    std::string         SVTI_toUnitRefType(SIM_VALREP_TYPE_I_ALL svti);
    std::string         SVTI_toUnitPtrType(SIM_VALREP_TYPE_I_ALL svti);

    int                 getSvtMaxBitSize (SIM_VALREP_TYPE_ALL svt);
    int                 getArrSize       (int size);

    SIM_VALREP_TYPE_I_ALL cvtValRepType(SIM_VALREP_TYPE_ALL svt);



    struct SIM_VALREP_TYPE_ALL{
        SIM_VALREP_TYPE type;
        int             subType; //// it is used when type is exceed 64 bit, it will store the size of array

        explicit SIM_VALREP_TYPE_ALL(int bitSize):
        type(getMatchSVT(bitSize)),
        subType(-1){
            if (type == SVT_U64M){
                subType = getArrSize(bitSize);
            }
        }

        explicit SIM_VALREP_TYPE_ALL():
        type(SVT_ERR),
        subType(-1){}

        bool operator == (const SIM_VALREP_TYPE_ALL& rhs) const{
            return (type == rhs.type) && (subType == rhs.subType);
        }



    };

    struct SIM_VALREP_TYPE_I_ALL{
        SIM_VALREP_TYPE_INT type;
        int                 subType; //// it is used when type is exceed 64 bit, it will store the size of array

        explicit SIM_VALREP_TYPE_I_ALL(int bitSize):
        type(getMatchSVTI(bitSize)),
        subType(-1){
            if (static_cast<int>(type) == static_cast<int>(SVT_U64M)){
                subType = getArrSize(bitSize);
            }
        }

        explicit SIM_VALREP_TYPE_I_ALL():
        type(SVTI_ERR),
        subType(-1){}

        explicit SIM_VALREP_TYPE_I_ALL(SIM_VALREP_TYPE_INT t, int st):
        type(t),
        subType(st){
            assert((t != SVTI_64M) && (st == -1));
        }

        bool operator == (const SIM_VALREP_TYPE_I_ALL& rhs) const{
            return (type == rhs.type) && (subType == rhs.subType);
        }


    };



    struct ValR{
        SIM_VALREP_TYPE_ALL _valType = SIM_VALREP_TYPE_ALL();
        int _size      = -1 ;
        std::string _data = "unused";


        ValR(): _valType(){}
        ValR(SIM_VALREP_TYPE_ALL valType, int size, std::string data):
        _valType(valType),
        _size(size),
        _data(std::move(data)){}

        ValR(SIM_VALREP_TYPE_ALL valType, int size):
        _valType(valType),
        _size(size){}

        int                 getSize() const {return _size;}
        SIM_VALREP_TYPE_ALL getType() const {return _valType;}

        ////////// bitwise
        ValR operator &  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "( " + _data  + " & " +  rhs._data + ")"}; }
        ValR operator |  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "( " + _data  + " | " +  rhs._data + ")"}; }
        ValR operator ^  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "( " + _data  + " ^ " +  rhs._data + ")"}; }
        ValR operator ~  ()                const{                             return {_valType, _size, "(~" + _data + ")"};}
        ValR operator << (const ValR& rhs) const{                             return {_valType, _size, "(" + _data +  " << " + rhs._data + ")"};}
        ValR operator >> (const ValR& rhs) const{                             return {_valType, _size, "(" + _data +  " >> " + rhs._data + ")"};}
        ////////// logical
        ValR operator && (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + _data +  "&&" +  rhs._data + ")"};}
        ValR operator || (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + _data +  "||" +  rhs._data + ")"};}
        ValR operator !  ()                const{                             return {SIM_VALREP_TYPE_ALL(1), 1, "(!" + _data + ")"};}
        ////////// relational
        ValR operator == (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + _data + "==" +  rhs._data + ")"};}
        ValR operator != (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + _data + "!=" +  rhs._data + ")"};}
        ValR operator <  (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + _data + "< " +  rhs._data + ")"};}
        ValR operator <= (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + _data + "<=" +  rhs._data + ")"};}
        ValR operator >  (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + _data + "> " +  rhs._data + ")"};}
        ValR operator >= (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + _data + ">=" +  rhs._data + ")"};}
        ////////// relational with sign
        ValR slt (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + castToSignStr() + "<" +  rhs.castToSignStr() + ")"};}
        ValR sgt (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + castToSignStr() + ">" +  rhs.castToSignStr() + ")"};}

        ////////// operation
        ValR operator +  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "+" + rhs._data + ")"};}
        ValR operator -  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "-" + rhs._data + ")"};}
        ValR operator *  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "*" + rhs._data + ")"};}
        ValR operator /  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "/" + rhs._data + ")"};}
        ValR operator %  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "%" + rhs._data + ")"};}


        [[nodiscard]]
        std::string buildMask (const int size, const int start = 0) const{
            if(_valType.type == SVT_U64M){
                return "(" + SVT_toUnitType(_valType) + "(0).buildMask(" + std::to_string(size) + "," + std::to_string(start) + "))";
            }
            assert( (size + start) <= getSvtMaxBitSize(_valType));
            ull value = (size == bitSizeOfUll) ? (INT64_MAX << start) : (((1ULL << size) - 1) << start);
            return "(static_cast<" + SVT_toUnitType(_valType) +">(" + cvtNum2HexStr(value) + "))";

        }

        [[nodiscard]]
        std::string buildZero() const{
            if(_valType.type == SVT_U64M){
                return "(" + SVT_toUnitType(_valType) + "(0))";
            }
            return "(static_cast<" + SVT_toUnitType(_valType) +">(0))";
        }

        ////// bit extend
        ValR ext(int size) const{

            ValR des(SIM_VALREP_TYPE_ALL(size), size);
            des.setData("("+_data + " ? " + des.buildMask(size) + " : " + des.buildZero() + ")");
            return des;

        }

        ValR enforceSize() const{
            return {_valType, _size,"(" + _data + " & " + buildMask(_size) + ")"};
        }

        ValR clear (Slice sl) const{

            std::string mask = buildMask(sl.getSize(), sl.start);

            return {_valType, _size,"("+_data +"& (~" + mask + "))"};
        }

        /////// do it on slice and shift


        ValR eq(const ValR& rhs){
            assert(_valType == rhs._valType);
            assert(_size    == rhs._size);
            return ValR(_valType, _size, _data + " = " + rhs._data);
        }

        ValR partialOr(const ValR& rhs){ //// this is like or but not check actual size only check simtype
            assert(_valType == rhs._valType);
            return {_valType, _size, "( " + _data  + " | " +  rhs._data + ")"};
        }


        [[nodiscard]]
        ValR slice (Slice sl) const{
            assert( (sl.start >= 0));
            int  targetSize = sl.getSize();
            assert(targetSize <= _size);
            if (sl.start == 0){
                return { _valType, _size,"(" + _data +"&" + buildMask(targetSize)+")"};
            }else{
                return { _valType, _size,"(("+_data+">>"+std::to_string(sl.start)+")&" + buildMask(targetSize)+")"};
            }

        }

        ValR shift(const int start) const{
            assert( (start + _size) <= getSvtMaxBitSize(_valType));
            if (start == 0){
                return *this;
            }

            return {_valType, _size,"(" +_data + "<<" + std::to_string(start) + ")"};
        }

        ValR castBase(SIM_VALREP_TYPE_ALL desVt, int size) const{

            if ((desVt.type == SVT_U64M)  && (_valType.type != SVT_U64M)){
                return {desVt, size, "UintX<" + std::to_string(desVt.subType) + ">(" + _data + ")"};
            }

            return {desVt, size, "static_cast<" + SVT_toUnitType(desVt) + ">(" + _data + ")"};
        }

        ValR cast(SIM_VALREP_TYPE_ALL desVt, int size) const{
            if (_valType == desVt){
                return {desVt, size, _data};
            }
            return castBase(desVt, size);
        }

        ////// This is supposed to use with the internal only
        std::string castToSignStr() const{

            SIM_VALREP_TYPE_I_ALL svti = cvtValRepType(_valType);
            mfAssert(svti.type <= SVTI_64M, "for now sign value conversion is not available");

            return "static_cast<" + SVTI_toUnitType(svti) + ">(" + _data + ")";


        }

        void setData(std::string dayta){
            _data = std::move(dayta);
        }

        [[nodiscard]]
        std::string toString() const{
            return _data;
        }

        std::string getData() const{
            return _data;
        }

        std::string getRefData() const{
            return "&" + _data;
        }

        [[nodiscard]]
        std::string buildVar(ull initVal) const{
            return SVT_toUnitType(_valType) + " " + _data + " = " + std::to_string(initVal);
        }


        [[nodiscard]]
        std::string buildVar() const{
            std::string buffer =  SVT_toUnitType(_valType) + " " + _data;
            return buffer;
        }

        [[nodiscard]] /// used for build linking variable
        std::string buildVarRef(const std::string& newName ) const{
            return SVT_toUnitRefType(_valType) + " " + newName + " = " + getData();
        }

        [[nodiscard]]
        std::string buildMemVar(ull depthSize) const{
            return SVT_toUnitType(_valType) + " " + _data + "["+ std::to_string(depthSize) + "]";
        }

        [[nodiscard]] /// used for build linking variable
        std::string buildMemVarPtr(const std::string& newName) const{
            /////// getdata() should return pointer
            return SVT_toUnitPtrType(_valType) + " " + newName + " = " + getData();
        }

        ValR index(ValR idx){
            return {_valType, _size, _data + "[" + idx.toString() +"]"};
        }


    };

}

#endif //SIMVALTYPE_H
