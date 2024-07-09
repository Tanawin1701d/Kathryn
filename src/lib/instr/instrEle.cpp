//
// Created by tanawin on 9/7/2024.
//
#include "instrEle.h"

#include <utility>
#include "util/str/strUtil.h"

namespace kathryn{


    RegIdxAsm::RegIdxAsm(token& tk){
        std::string asmType = tk.splitedValue[TOKEN_TYPE_IDX].
                               substr(TOKEN_TYPE_START_IDX,
                                      TOKEN_TYPE_END_IDX);
        if (asmType == TOKEN_TYPE_SRC){
            isRead = true;
        }
        if (asmType == TOKEN_TYPE_DES){
            isRead = false;
        }
        _regCnt = std::stoi(tk.splitedValue[TOKEN_TYPE_IDX].
                            substr(TOKEN_REG_NUM_START_IDX));
        _srcSlice = tk.sl;

        assert(_regCnt >= 0);
        assert(_srcSlice.checkValidSlice());
    }

    void ImmAsm::addImmMeta(token& tk){
        assert(tk.splitedValue.size() == (TOKEN_FILLB_IDX+1));
        assert(tk.sl.checkValidSlice());
        _srcSlices.push_back(tk.sl);
        // fill to [a, b)
        int fillA = std::stoi(tk.splitedValue[TOKEN_FILLA_IDX]);
        int fillB = std::stoi(tk.splitedValue[TOKEN_FILLB_IDX]);
        Slice fillSl = {fillA, fillB};
        assert(fillSl.checkValidSlice());
        _desSlices.push_back(fillSl);
    }

    void OpcodeAsm::addOpMeta(token& tk){
        assert(tk.sl.checkValidSlice());
        assert(tk.value.size() == tk.sl.getSize());
        _srcSlices.push_back(tk.sl);
        _expectValues.push_back(tk.value);

    }





    /***
     *
     * TOKEN
     *
     */

    void token::finalToken(){
        /////// sl.start is filled you must compute sl.stop
        try{
            splitedValue = splitStr(value, '-');
            if(splitedValue.size() == 1){
                sl.start = sl.stop - ((int)value.size());
            }else{
                sl.start = sl.stop - std::stoi(splitedValue[0]);
            }
            assert(sl.checkValidSlice());

        }catch (...){
            assert(false); /// can't determine size
        }
    }

    /***
     *
     * Instruction type
     *
     */

    InstrType::InstrType(std::vector<token> tokens):
    _tokens(std::move(tokens)){}


    void InstrType::intepretToken(){

        for (token tk: _tokens){
            std::vector<std::string> dec = tk.splitedValue;
            Slice                     sl = tk.sl;
            assert(!dec.empty());
            if (dec.size() == 1){
                /////// it is op
                _opAsm.addOpMeta(tk);
                return;
            }
            if(dec[TOKEN_ASM_TYPE_IDX][0] == TOKEN_ASM_TYPE_REG_IDX){
                RegIdxAsm worker(tk);
                if (worker.isRead){_srcRegAsms.push_back(worker);}
                else              {_desRegAsms.push_back(worker);}
                return;
            }
            if(dec[TOKEN_ASM_TYPE_IDX][0] == TOKEN_ASM_TYPE_IMM_IDX){
                _immAsm.addImmMeta(tk);
                return;
            }
        }

    }


    void InstrType::doAllAsm(){
        /** declare assignment*/
        for (RegIdxAsm& srcRegAsm: _srcRegAsms){
            srcRegAsm.doAsm();
        }
        for (RegIdxAsm& desRegAsm: _desRegAsms){
            desRegAsm.doAsm();
        }
        _opAsm .doAsm();
        _immAsm.doAsm();
    }

}