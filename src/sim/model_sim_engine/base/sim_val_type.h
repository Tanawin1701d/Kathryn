//
// Created by tanawin on 23/7/2024.
//

#ifndef SIMVALTYPE_H
#define SIMVALTYPE_H

#include "string"
#include "cassert"
#include "utility"
#include "model/hw_component/abstract/identifiable.h"
#include "model/hw_component/abstract/slice.h"
#include "util/numberic/num_convert.h"

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
    SIM_VALREP_TYPE     get_match_svt      (int size);
    SIM_VALREP_TYPE_INT get_match_svti     (int size);

    std::string         SVT_toUnitType   (SIM_VALREP_TYPE_ALL svt);
    std::string         SVTI_toUnitType  (SIM_VALREP_TYPE_I_ALL svti);

    std::string         SVT_toUnitRefType(SIM_VALREP_TYPE_ALL svt);
    std::string         SVT_toUnitPtrType(SIM_VALREP_TYPE_ALL svt);
    std::string         SVTI_toUnitRefType(SIM_VALREP_TYPE_I_ALL svti);
    std::string         SVTI_toUnitPtrType(SIM_VALREP_TYPE_I_ALL svti);

    int                 get_svt_max_bit_size (SIM_VALREP_TYPE_ALL svt);
    int                 get_arr_size       (int size);

    SIM_VALREP_TYPE_I_ALL cvt_val_rep_type(SIM_VALREP_TYPE_ALL svt);



    struct SIM_VALREP_TYPE_ALL{
        SIM_VALREP_TYPE type;
        int             sub_type; //// it is used when type is exceed 64 bit, it will store the size of array

        explicit SIM_VALREP_TYPE_ALL(int bit_size):
        type(get_match_svt(bit_size)),
        sub_type(-1){
            if (type == SVT_U64M){
                sub_type = get_arr_size(bit_size);
            }
        }

        explicit SIM_VALREP_TYPE_ALL():
        type(SVT_ERR),
        sub_type(-1){}

        bool operator == (const SIM_VALREP_TYPE_ALL& rhs) const{
            return (type == rhs.type) && (sub_type == rhs.sub_type);
        }



    };

    struct SIM_VALREP_TYPE_I_ALL{
        SIM_VALREP_TYPE_INT type;
        int                 sub_type; //// it is used when type is exceed 64 bit, it will store the size of array

        explicit SIM_VALREP_TYPE_I_ALL(int bit_size):
        type(get_match_svti(bit_size)),
        sub_type(-1){
            if (static_cast<int>(type) == static_cast<int>(SVT_U64M)){
                sub_type = get_arr_size(bit_size);
            }
        }

        explicit SIM_VALREP_TYPE_I_ALL():
        type(SVTI_ERR),
        sub_type(-1){}

        explicit SIM_VALREP_TYPE_I_ALL(SIM_VALREP_TYPE_INT t, int st):
        type(t),
        sub_type(st){
            assert((t != SVTI_64M) && (st == -1));
        }

        bool operator == (const SIM_VALREP_TYPE_I_ALL& rhs) const{
            return (type == rhs.type) && (sub_type == rhs.sub_type);
        }


    };



    struct ValR{
        SIM_VALREP_TYPE_ALL _valType = SIM_VALREP_TYPE_ALL();
        int _size      = -1 ;
        std::string _data = "unused";


        ValR(): _valType(){}
        ValR(SIM_VALREP_TYPE_ALL val_type, int size, std::string data):
        _valType(val_type),
        _size(size),
        _data(std::move(data)){}

        ValR(SIM_VALREP_TYPE_ALL val_type, int size):
        _valType(val_type),
        _size(size){}

        int                 get_size() const {return _size;}
        SIM_VALREP_TYPE_ALL get_type() const {return _valType;}

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
        ValR slt (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + cast_to_sign_str() + "<" +  rhs.cast_to_sign_str() + ")"};}
        ValR sgt (const ValR& rhs) const{ assert(_size == rhs._size); return {SIM_VALREP_TYPE_ALL(1), 1, "(" + cast_to_sign_str() + ">" +  rhs.cast_to_sign_str() + ")"};}

        ////////// operation
        ValR operator +  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "+" + rhs._data + ")"};}
        ValR operator -  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "-" + rhs._data + ")"};}
        ValR operator *  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "*" + rhs._data + ")"};}
        ValR operator /  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "/" + rhs._data + ")"};}
        ValR operator %  (const ValR& rhs) const{ assert(_size == rhs._size); return {_valType, _size, "(" + _data + "%" + rhs._data + ")"};}


        [[nodiscard]]
        std::string build_mask (const int size, const int start = 0) const{
            if(_valType.type == SVT_U64M){
                return "(" + SVT_toUnitType(_valType) + "(0).build_mask(" + std::to_string(size) + "," + std::to_string(start) + "))";
            }
            assert( (size + start) <= get_svt_max_bit_size(_valType));
            ull value = (size == bit_size_of_ull) ? (INT64_MAX << start) : (((1ULL << size) - 1) << start);
            return "(static_cast<" + SVT_toUnitType(_valType) +">(" + cvt_num2_hex_str(value) + "))";

        }

        [[nodiscard]]
        std::string build_zero() const{
            if(_valType.type == SVT_U64M){
                return "(" + SVT_toUnitType(_valType) + "(0))";
            }
            return "(static_cast<" + SVT_toUnitType(_valType) +">(0))";
        }

        ////// bit extend
        ValR ext(int size) const{

            ValR des(SIM_VALREP_TYPE_ALL(size), size);
            des.set_data("("+_data + " ? " + des.build_mask(size) + " : " + des.build_zero() + ")");
            return des;

        }

        ValR enforce_size() const{
            return {_valType, _size,"(" + _data + " & " + build_mask(_size) + ")"};
        }

        ValR clear (Slice sl) const{

            std::string mask = build_mask(sl.get_size(), sl.start);

            return {_valType, _size,"("+_data +"& (~" + mask + "))"};
        }

        /////// do it on slice and shift


        ValR eq(const ValR& rhs){
            assert(_valType == rhs._valType);
            assert(_size    == rhs._size);
            return ValR(_valType, _size, _data + " = " + rhs._data);
        }

        ValR partial_or(const ValR& rhs){ //// this is like or but not check actual size only check simtype
            assert(_valType == rhs._valType);
            return {_valType, _size, "( " + _data  + " | " +  rhs._data + ")"};
        }


        [[nodiscard]]
        ValR slice (Slice sl) const{
            assert( (sl.start >= 0));
            int  target_size = sl.get_size();
            assert(target_size <= _size);
            if (sl.start == 0){
                return { _valType, _size,"(" + _data +"&" + build_mask(target_size)+")"};
            }else{
                return { _valType, _size,"(("+_data+">>"+std::to_string(sl.start)+")&" + build_mask(target_size)+")"};
            }

        }

        ValR shift(const int start) const{
            assert( (start + _size) <= get_svt_max_bit_size(_valType));
            if (start == 0){
                return *this;
            }

            return {_valType, _size,"(" +_data + "<<" + std::to_string(start) + ")"};
        }

        ValR cast_base(SIM_VALREP_TYPE_ALL des_vt, int size) const{

            if ((des_vt.type == SVT_U64M)  && (_valType.type != SVT_U64M)){
                return {des_vt, size, "UintX<" + std::to_string(des_vt.sub_type) + ">(" + _data + ")"};
            }

            return {des_vt, size, "static_cast<" + SVT_toUnitType(des_vt) + ">(" + _data + ")"};
        }

        ValR cast(SIM_VALREP_TYPE_ALL des_vt, int size) const{
            if (_valType == des_vt){
                return {des_vt, size, _data};
            }
            return cast_base(des_vt, size);
        }

        ////// This is supposed to use with the internal only
        std::string cast_to_sign_str() const{

            SIM_VALREP_TYPE_I_ALL svti = cvt_val_rep_type(_valType);
            mf_assert(svti.type <= SVTI_64M, "for now sign value conversion is not available");

            return "static_cast<" + SVTI_toUnitType(svti) + ">(" + _data + ")";


        }

        void set_data(std::string dayta){
            _data = std::move(dayta);
        }

        [[nodiscard]]
        std::string to_string() const{
            return _data;
        }

        std::string get_data() const{
            return _data;
        }

        std::string get_ref_data() const{
            return "&" + _data;
        }

        [[nodiscard]]
        std::string build_var(ull init_val) const{
            return SVT_toUnitType(_valType) + " " + _data + " = " + std::to_string(init_val);
        }


        [[nodiscard]]
        std::string build_var() const{
            std::string buffer =  SVT_toUnitType(_valType) + " " + _data;
            return buffer;
        }

        [[nodiscard]] /// used for build linking variable
        std::string build_var_ref(const std::string& new_name ) const{
            return SVT_toUnitRefType(_valType) + " " + new_name + " = " + get_data();
        }

        [[nodiscard]]
        std::string build_mem_var(ull depth_size) const{
            return SVT_toUnitType(_valType) + " " + _data + "["+ std::to_string(depth_size) + "]";
        }

        [[nodiscard]] /// used for build linking variable
        std::string build_mem_var_ptr(const std::string& new_name) const{
            /////// getdata() should return pointer
            return SVT_toUnitPtrType(_valType) + " " + new_name + " = " + get_data();
        }

        ValR index(ValR idx){
            return {_valType, _size, _data + "[" + idx.to_string() +"]"};
        }


    };

}

#endif //SIMVALTYPE_H
