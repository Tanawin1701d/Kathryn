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

    constexpr char SIM_VALREP_BASE_NAME [] = "ValR";

    enum SIM_VALREP_TYPE{
        SVT_U8 = 0,
        SVT_U16 = 1,
        SVT_U32 = 2,
        SVT_U64 = 3,
        SVT_U64M = 4,
        SVT_ERR  = 5,
        SVT_CNT  = 6
    };
    std::string SVT_toUnitType(SIM_VALREP_TYPE svt);
    std::string SVT_toType(SIM_VALREP_TYPE svt);

    SIM_VALREP_TYPE getMatchSVT(Operable* opr);
    SIM_VALREP_TYPE getMatchSVT(int size);
    int             getSvtMaxBitSize(SIM_VALREP_TYPE svt);

    struct ValR{
        SIM_VALREP_TYPE _valType = SVT_ERR;
        int _size = -1 ;
        std::string _data = "unused";


        ValR(): _valType(){}
        ValR(SIM_VALREP_TYPE valType, int size, std::string data):
        _valType(valType),
        _size(size),
        _data(std::move(data)){}

        ValR(SIM_VALREP_TYPE valType, int size):
        _valType(valType),
        _size(size){}

        ////////// bitwise
        ValR operator &  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "( " + _data  + " & " +  rhs._data + ")"}; }
        ValR operator |  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "( " + _data  + " | " +  rhs._data + ")"}; }
        ValR operator ^  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "( " + _data  + " ^ " +  rhs._data + ")"}; }
        ValR operator ~  ()                const{                             return {_valType, _size, "(~" + _data + ")"};}
        ValR operator << (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data +  " << " + rhs._data + ")"};}
        ValR operator >> (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data +  " >> " + rhs._data + ")"};}
        ////////// logical
        ValR operator && (const ValR& rhs) const{ assert(_size == rhs._size); return {SVT_U8, 1, "(" + _data +  "&&" +  rhs._data + ")"};}
        ValR operator || (const ValR& rhs) const{ assert(_size == rhs._size); return {SVT_U8, 1, "(" + _data +  "||" +  rhs._data + ")"};}
        ValR operator !  ()                const{                             return {SVT_U8, 1, "(!" + _data + ")"};}
        ////////// relational
        ValR operator == (const ValR& rhs) const{ assert(_size == rhs._size); return {SVT_U8, 1, "(" + _data + "==" +  rhs._data + ")"};}
        ValR operator != (const ValR& rhs) const{ assert(_size == rhs._size); return {SVT_U8, 1, "(" + _data + "!=" +  rhs._data + ")"};}
        ValR operator <  (const ValR& rhs) const{ assert(_size == rhs._size); return {SVT_U8, 1, "(" + _data + "< " +  rhs._data + ")"};}
        ValR operator <= (const ValR& rhs) const{ assert(_size == rhs._size); return {SVT_U8, 1, "(" + _data + "<=" +  rhs._data + ")"};}
        ValR operator >  (const ValR& rhs) const{ assert(_size == rhs._size); return {SVT_U8, 1, "(" + _data + "> " +  rhs._data + ")"};}
        ValR operator >= (const ValR& rhs) const{ assert(_size == rhs._size); return {SVT_U8, 1, "(" + _data + ">=" +  rhs._data + ")"};}

        ////////// relational
        ValR operator +  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "+" + rhs._data + ")"};}
        ValR operator -  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "-" + rhs._data + ")"};}
        ValR operator *  (const ValR& rhs) const{ assert(false);}
        ValR operator /  (const ValR& rhs) const{ assert(false);}
        ValR operator %  (const ValR& rhs) const{ assert(false);}


        [[nodiscard]]
        std::string buildMask (const int size, const int start = 0) const{
            assert( (size + start) < getSvtMaxBitSize(_valType));
            ull value = (size == bitSizeOfUll) ? (INT64_MAX << start) : ((1ULL << size - 1) << start);
            return "(static_cast<" + SVT_toUnitType(_valType) +">(" + cvtNum2HexStr(value) + "))";

        }

        [[nodiscard]]
        std::string buildZero() const{
            return "(static_cast<" + SVT_toUnitType(_valType) +">(0))";
        }

        ////// bit extend
        ValR ext(int size) const{

            ValR des(getMatchSVT(size), size);
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


        [[nodiscard]]
        ValR slice (Slice sl) const{
            assert( (sl.start >= 0));
            int  targetSize = sl.getSize();
            assert(targetSize <= _size);
            return { _valType, _size,"(("+_data+">>"+std::to_string(sl.start)+")&" + buildMask(targetSize)+")"};
        }

        ValR shift(const int start) const{
            assert( (start + _size) < getSvtMaxBitSize(_valType));
            return {_valType, _size,"(" +_data + "<<" + std::to_string(start) + ")"};
        }

        ValR cast(SIM_VALREP_TYPE desVt, int size) const{
            if (_valType == desVt){
                return {desVt, size, _data};
            }
            return {desVt, size, "static_cast<" + SVT_toUnitType(desVt) + ">(" + _data + ")"};
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





        [[nodiscard]]
        std::string buildVar(ull initVal) const{
            return SVT_toUnitType(_valType) + " " + _data + " = " + std::to_string(initVal);
        }


        [[nodiscard]]
        std::string buildVar() const{
            std::string buffer =  SVT_toUnitType(_valType) + " " + _data;
            return buffer;
        }

        std::string buildMemVar(ull depthSize) const{
            return SVT_toUnitType(_valType) + " " + _data + "["+ std::to_string(depthSize) + "]";
        }

        ValR index(ValR idx){
            return {_valType, _size, _data + "[" + idx.toString() +"]"};
        }


    };

}

#endif //SIMVALTYPE_H
