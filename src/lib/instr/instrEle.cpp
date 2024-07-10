//
// Created by tanawin on 9/7/2024.
//
#include "instrEle.h"
#include "instrBase.h"
#include <utility>
#include "util/str/strUtil.h"

namespace kathryn{


    RegIdxAsm::RegIdxAsm(const token& tk){
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

    void ImmAsm::addImmMeta(const token& tk){
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

    void OpcodeAsm::addOpMeta(const token& tk){
        assert(tk.sl.checkValidSlice());
        assert(tk.value.size() == tk.sl.getSize());
        _srcSlices.push_back(tk.sl);
        _expectValues.push_back(tk.value);

    }

    void OpcodeAsm::setUopMaster(UopAsm* uopMaster){
        assert(uopMaster != nullptr);
        _uopMaster = uopMaster;
    }

    std::string OpcodeAsm::getSumOpcode() const{
        int instrSize = _master->getInstrSize();
        std::string result = genConString('0', instrSize);

        assert(_srcSlices.size() == _expectValues.size());

        for (int iter = 0; iter < _srcSlices.size(); iter++){
            int curBitSet = _srcSlices[iter].start;
            assert(_srcSlices[iter].getSize() == _expectValues[iter].size());
            assert(_srcSlices[iter].checkValidSlice() &&
                   _srcSlices[iter].stop <= instrSize
            );

            for (int expectValIdx = (int)_expectValues[iter].size()-1;
                     expectValIdx >= 0;
                     expectValIdx--
                ){
                //// v------------ last bit
                int actualStart = (instrSize - 1) - curBitSet;
                result[actualStart] = _expectValues[iter][expectValIdx];
                curBitSet++;
            }
        }
        return result;
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

    UopAsm::UopAsm(InstrRepo* master,
                   std::vector<token> tokens,std::string uopName,
                   int typeIdx, int uopIdx):
    _master (master),
    _uopName(std::move(uopName)),
    _typeIdx(typeIdx),
    _uopIdx (uopIdx),
    _tokens (std::move(tokens))
    { assert(_master != nullptr);}


    void UopAsm::intepretToken(){

        for (const token& tk: _tokens){
            std::vector<std::string> dec = tk.splitedValue;
            assert(!dec.empty());
            if (dec.size() == 1){
                /////// it is op
                _opAsm.addOpMeta(tk);
                continue;
            }
            if(dec[TOKEN_ASM_TYPE_IDX][0] == TOKEN_ASM_TYPE_REG_IDX){
                RegIdxAsm worker(tk);
                if (worker.isRead){_srcRegAsms.push_back(worker);}
                else              {_desRegAsms.push_back(worker);}
                continue;
            }
            if(dec[TOKEN_ASM_TYPE_IDX][0] == TOKEN_ASM_TYPE_IMM_IDX){
                _immAsm.addImmMeta(tk);
            }
        }
    }

    void UopAsm::assignMasterToAsm(){
        for (RegIdxAsm& srcRegAsm: _srcRegAsms){
            srcRegAsm.setMaster(_master);
        }
        for (RegIdxAsm& desRegAsm: _desRegAsms){
            desRegAsm.setMaster(_master);
        }
        _opAsm .setMaster(_master);
        _opAsm.setUopMaster(this);
        _immAsm.setMaster(_master);
    }

    void UopAsm::startGetSumOpcode(){
        _flattedOpcode = _opAsm.getSumOpcode();
    }

    void UopAsm::doAllAsm(){
        bool effSrc[_master->getAmtSrcReg()] = {};
        bool effDes[_master->getAmtDesReg()] = {};
        /** declare assignment*/
        for (RegIdxAsm& srcRegAsm: _srcRegAsms){
            srcRegAsm.doAsm();
            effSrc[srcRegAsm._regCnt] = true; //// mark it is used
        }
        for (RegIdxAsm& desRegAsm: _desRegAsms){
            desRegAsm.doAsm();
            effDes[desRegAsm._regCnt] = true;
        }
        _opAsm .doAsm();
        _immAsm.doAsm();

    }

}