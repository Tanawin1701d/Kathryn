//
// Created by tanawin on 9/7/2024.
//
#include "instrEle.h"
#include "instrBase.h"
#include <utility>
#include <model/hwComponent/expression/nest.h>

#include "util/str/strUtil.h"

namespace kathryn{

    /**
     *
     * reg idx asm
     *
     ***/

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
        if ( isRead){ assert(_regCnt < _master->getAmtSrcReg());}
        if (!isRead){ assert(_regCnt < _master->getAmtDesReg());}
        assert(_srcSlice.stop <= _master->getOprSize());
        assert(_srcSlice.checkValidSlice());
    }

    void RegIdxAsm::doAsm(){
        assert(_master != nullptr);
        OPR_HW& opr = isRead ? _master->getSrcReg(_regCnt)
                             : _master->getDesReg(_regCnt);
        auto slicedRegFileIdx = _master->getInstrOpr()->doSlice(_srcSlice);
        opr.setOnlyIndex(slicedRegFileIdx);
    }


    /**
     *
     * imm idx asm
     *
     ***/

    void ImmAsm::addImmMeta(const token& tk){
        assert(tk.splitedValue.size() == (TOKEN_FILLB_IDX+1));
        assert(tk.sl.checkValidSlice());

        // fill to [a, b)
        _regCnt   = std::stoi(tk.splitedValue[TOKEN_TYPE_IDX].substr(TOKEN_TYPE_SIZE)); //// start number to end
        int fillA = std::stoi(tk.splitedValue[TOKEN_FILLA_IDX]);
        int fillB = std::stoi(tk.splitedValue[TOKEN_FILLB_IDX]);
        Slice fillSl = {fillA, fillB};
        assert(fillSl.checkValidSlice());
        immSlicer.push_back({
            false,
            tk.sl,
            fillSl,
            _master->getInstrOpr()->doSlice(tk.sl)
        });

        if ( (tk.splitedValue.size() > TOKEN_FILLEXT_IDX) &&
             (tk.splitedValue[TOKEN_FILLEXT_IDX][0] == ZEROEXTEND)){
            _signedExtend = false;
        }
    }

    void ImmAsm::doAsm(){

        ////// start sort first
        std::sort(immSlicer.begin(), immSlicer.end());
        assert(!immSlicer.empty());
        std::vector<IterImm> arrangedOpr;
        ///// arrange all imm to proper slot and make dummy if there is no
        /// imm to fill that slot fill it with dummy initialize the dummy
        int curStartIdx = 0;
        for(int idx = 0; idx < immSlicer.size(); idx++){
            IterImm& iterImm = immSlicer[idx];
            if (curStartIdx != iterImm.desSlice.start){
                arrangedOpr.emplace_back(true,{}, {curStartIdx, iterImm.desSlice.start}, nullptr);
            }
            arrangedOpr.push_back(iterImm);
            curStartIdx = iterImm.desSlice.stop;
        }
        assert(!arrangedOpr.empty());
        assert(!arrangedOpr.rbegin()->needDummySrc);
        if (curStartIdx != _master->getOprSize()){
            arrangedOpr.emplace_back(true, {}, {curStartIdx, _master->getOprSize()}, nullptr);
        }
        curStartIdx = _master->getOprSize();

        //// initialize the dummy

        for(int idx = 0; idx < (((int)arrangedOpr.size())-1); idx++){
            if (arrangedOpr[idx].needDummySrc){
                arrangedOpr[idx].sliced =
                    &makeOprVal("immFill", arrangedOpr[idx].desSlice.getSize(), 0);
            }
        }

        //// do extend bit if there needed
        if(arrangedOpr.rbegin()->needDummySrc){
            assert(arrangedOpr.size() >= 2);
            IterImm& lastIter    = *arrangedOpr.rbegin();
            IterImm& srcToExtend = *(arrangedOpr.rbegin()+1);
            if (_signedExtend){
                Operable* srcOpr = srcToExtend.sliced;
                int srcSize = srcOpr->getOperableSlice().getSize();
                Operable* onlylastBit = srcOpr->doSlice({srcSize-1, srcSize});
                lastIter.sliced  = &onlylastBit->extB(lastIter.desSlice.getSize());
            }else{
                lastIter.sliced  = &makeOprVal("lastImmFIll", lastIter.desSlice.getSize(), 0);
            }
        }


        ///// do nest
        std::vector<NestMeta> metas;
        for(IterImm imm: immSlicer){
            NestMeta meta = {imm.sliced, nullptr};
            metas.push_back(meta);
        }
        nest& nested = gManInternal(metas);
        //// put it to operand
        OPR_HW& opr = _master->getSrcReg(_regCnt);
        opr.setImm(&nested);
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

    UopAsm::UopAsm(MOP* master,
                   std::vector<token> tokens,
                   std::string uopName,
                   int uopIdx):
    _mopMaster    (master),
    _uopTokens (std::move(tokens)),
    _uopName   (std::move(uopName)),
    _uopIdx    (uopIdx)
    { assert(_mopMaster != nullptr);}

    void UopAsm::addUopIdentToken(std::vector<token> tokens){
        _opTokens = std::move(tokens);
        assert(_opTokens.size() == _uopTokens.size());
    }

    void UopAsm::doAsm(){
        //// get hardware and set condition
        assert(!_mopMaster->_masterUopTokens.empty());
        assert(_mopMaster->_masterUopTokens.size() == _uopTokens.size());
        std::vector<Operable*> matchOpr;

        ///// genMatch expression
        for(int idx = 0; idx < (int)_mopMaster->_masterUopTokens.size(); idx++){
            Slice instrSlice = _mopMaster->_masterUopTokens[idx].sl;
            Operable* slicedInstr =
                _mopMaster->_master->getInstrOpr()->doSlice(instrSlice);
            ull cvtMatchStr = std::stoull(_uopTokens[idx].splitedValue[0], nullptr, 2);
            Val& matchVal = makeOprVal(_uopName + "_uopMc", instrSlice.getSize(), cvtMatchStr);

            matchOpr.push_back( &((*slicedInstr) == matchVal));
        }

        ////// genAll match is equal
        Operable* result = matchOpr[0];
        for (int idx = 1; idx < matchOpr.size(); idx++){
         result = &((*result) & (*matchOpr[idx]));
        }
        ////// assign to hardware
        int mopIdx = _mopMaster->_mopIdx;
        InstrRepo* repo = _mopMaster->_master;
        repo->getOp(mopIdx).setUop(_uopIdx, result);
    }


    /***
     *
     *
     * mop
     *
     *
     */

    MOP::MOP(InstrRepo* master,
             std::vector<token>& masterTokens,
             std::string mopName,
            int mopIdx
    ):
    _master(master),
    _masterTokens(masterTokens),
    _mopName(std::move(mopName)),
    _mopIdx(mopIdx)
    { assert(master != nullptr);}


    void MOP::interpretMasterToken(){

        for (const token& tk: _masterTokens){
            std::vector<std::string> dec = tk.splitedValue;
            assert(!dec.empty());
            if (dec.size() == 1){
                /////// it is op
                _opcodeTokens.push_back(tk);
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

            if(dec[TOKEN_ASM_TYPE_IDX][0] == TOKEN_ASM_TYPE_UOP_IDX){
                _masterUopTokens.push_back(tk);
            }
        }
    }

    void MOP::createUop(std::vector<token>& uopDataTokens, const std::string& uopName){
        assert(uopDataTokens.size() == _masterUopTokens.size());
        _uops.emplace_back(
            this,
            uopDataTokens,
            uopName,
            _uops.size());
    }

    void MOP::assignMaster(){
        ////// assign instr repo to all
        for (RegIdxAsm& regIdxAsm: _srcRegAsms){
            regIdxAsm.setMaster(_master);
        }
        for (RegIdxAsm& regIdxAsm: _desRegAsms){
            regIdxAsm.setMaster(_master);
        }
        _immAsm.setMaster(_master);
        for (UopAsm& uopAsm: _uops){
            uopAsm.setMaster(_master);
        }
    }


    void MOP::flattenMop(){
        _flattedInstr = genConString('0', _master->getInstrSize());
        for(token& tk: _opcodeTokens){
            int actualStrStart = _master->getOprSize() - tk.sl.stop;
            int actualStrStop  = _master->getOprSize() - tk.sl.start + 1;
            assert((actualStrStop-actualStrStart) == tk.sl.getSize());
            _flattedInstr.replace(
                actualStrStart,
                actualStrStop-actualStrStart,
                tk.splitedValue[0]
            );
        }
    }


    void MOP::doAllAsm(){

        bool effSrc[_master->getAmtSrcReg()] = {};
        bool effDes[_master->getAmtDesReg()] = {};
        /** declare assignment and check the effective hardware*/
        /** for register*/
        for (RegIdxAsm& srcRegAsm: _srcRegAsms){
            srcRegAsm.doAsm();
            effSrc[srcRegAsm._regCnt] = true; //// mark it is used
        }
        for (RegIdxAsm& desRegAsm: _desRegAsms){
            desRegAsm.doAsm();
            effDes[desRegAsm._regCnt] = true;
        }
        for (UopAsm& uopAsm:  _uops){
            uopAsm.doAsm();
        }
        _immAsm.doAsm();
        effSrc[_immAsm._regCnt] = true;

        /** disable unused hw*/
        unsetUnusedReg(effSrc, _master->getAmtSrcReg(), true);
        unsetUnusedReg(effDes, _master->getAmtDesReg(), false);


        for(int iterMopIdx = 0; iterMopIdx < _master->getAmtMop(); iterMopIdx++){
            if (iterMopIdx == _mopIdx){
                _master->getOp(_mopIdx).set();
            }else{
                _master->getOp(iterMopIdx).reset();
            }
        }


    }

    void MOP::unsetUnusedReg(const bool* eff, int size, bool isSrc){

        for (int regIdx = 0; regIdx < size; regIdx++){
            OPR_HW& regHw = isSrc ? _master->getSrcReg(regIdx) : _master->getDesReg(regIdx);
            if (!eff[regIdx]){
                regHw.reset();
            }
        }
    }


}