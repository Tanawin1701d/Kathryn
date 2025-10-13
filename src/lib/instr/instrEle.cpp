//
// Created by tanawin on 9/7/2024.
//
#include "instrEle.h"
#include "instrBase.h"
#include <utility>
#include "model/hwComponent/expression/nest.h"

#include "util/str/strUtil.h"

namespace kathryn{

    Operable* joinOpr(std::vector<Operable*> srcOprs, LOGIC_OP lop){
        assert(!srcOprs.empty());
        Operable* result = srcOprs[0];

        for (int i = 1; i < srcOprs.size(); i++){
            switch(lop){
            case BITWISE_AND:{
                result = &((*result) & (*srcOprs[i]));
                break;
            }
            case BITWISE_OR:{
                result = &((*result) | (*srcOprs[i]));
                break;
            }
            default: {assert(false);}
            }
        }
        return result;
    }

    /***
    *
    * TOKEN
    *
    */

    void token::splitToken(){
        splitedValue = splitStr(value, '-');
    }

    void token::sizeDet(int startBit){
        /////// sl.start is filled you must compute sl.stop
        try{
            assert(startBit >= 0);
            sl.start = startBit;
            if(splitedValue.size() == 1){
                sl.stop = sl.start + ((int)value.size()); //// <0000011>
            }else{
                sl.stop = sl.start  + std::stoi(splitedValue[0]); ///// <5-rd0> use 5
            }
            assert(sl.checkValidSlice());
            isSizeDet = true;
        }catch (...){
            assert(false); /// can't determine size
        }
    }


    //////////////////////////
    /// TOKEN_GRP
    //////////////////////////

    TOKEN_GRP::TOKEN_GRP(std::string rawValue, bool reqSizeDet){
        std::stack<token> tokenSt;
        /////// iterate all in string
        for (int idx = 0; idx < rawValue.size(); idx++){
            char bitChar = rawValue[idx];
            switch (bitChar){
            case '<':{tokenSt.emplace(); break;}
            case '>':{
                token topToken = tokenSt.top();
                topToken.splitToken();
                _tokens.push_back(topToken);
                tokenSt.pop();
                break;
            }
            default:{
                assert(!tokenSt.empty());
                tokenSt.top().addRawChar(bitChar); break;
            }
            }
        }

        if (reqSizeDet){
            int nextStartIdx = 0;
            for (auto tk = _tokens.rbegin(); tk != _tokens.rend(); tk++){
                tk->sizeDet(nextStartIdx);
                nextStartIdx += tk->sl.getSize();
            }
        }
        assert(tokenSt.empty());
    }

    /**
     *
     * reg idx asm
     *
     ***/

    RegIdxAsm::RegIdxAsm(InstrRepo* master, const token& tk): AsmWorker(master){
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
        OPR_HW& opr1 = isRead ? _master->getSrcReg(_regCnt)
                              : _master->getDesReg(_regCnt);
        auto slicedRegFileIdx = _master->getInstrOpr()->doSlice(_srcSlice);
        opr1.setOnlyIndex(slicedRegFileIdx);
    }


    /**
     *
     * imm idx asm
     *
     ***/

    void ImmAsm::addImmMeta(const token& tk){
        assert(tk.splitedValue.size() >= (TOKEN_FILLB_IDX+1));
        assert(tk.sl.checkValidSlice());

        // fill to [a, b)
        _regCnt   = std::stoi(tk.splitedValue[TOKEN_TYPE_IDX].substr(TOKEN_TYPE_SIZE)); //// start number to end
        int fillA = std::stoi(tk.splitedValue[TOKEN_FILLA_IDX]);
        int fillB = std::stoi(tk.splitedValue[TOKEN_FILLB_IDX]);
        Slice fillSl = {fillA, fillB};
        assert((fillB-fillA) == std::stoi(tk.splitedValue[0]));
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

        if (immSlicer.empty()){
            return;
        }

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
                arrangedOpr.push_back({true,{-1,-1}, {curStartIdx, iterImm.desSlice.start}, nullptr});
            }
            arrangedOpr.push_back(iterImm);
            curStartIdx = iterImm.desSlice.stop;
        }
        assert(!arrangedOpr.empty());
        assert(!arrangedOpr.rbegin()->needDummySrc);
        if (curStartIdx != _master->getOprSize()){
            arrangedOpr.push_back({true, {-1,-1}, {curStartIdx, _master->getOprSize()}, nullptr});
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
        std::vector<Operable*> metas;
        for(IterImm imm: arrangedOpr){
            metas.push_back(imm.sliced);
        }
        nest& nested = gManInternalReadOnly(metas);
        //// put it to operand
        OPR_HW& opr = _master->getSrcReg(_regCnt);
        opr.setImm(&nested);
    }



    /////////////////////////
    /// MatchSlaveVal
    /////////////////////////

    MatchSlaveVal::MatchSlaveVal(const std::string& matchRule,
                                 const std::string& matchVal,
                                 std::string uopName):
    _matchRule(matchRule, false),
    _matcher(matchVal, false),
    _effUopName(std::move(uopName)){}

    Operable* MatchSlaveVal::getActCond(
    std::map<std::string, token> slaveTokenMap,
    Operable* instr
    ){
        /////// traverse to our rule
        assert(_matchRule._tokens.size() == _matcher._tokens.size());
        std::vector<Operable*> activateConditions;
        for (int matchIdx = 0; matchIdx < _matchRule._tokens.size(); matchIdx++){
            token matchRuleToken = _matchRule._tokens[matchIdx];

            token matchValueToken = _matcher._tokens[matchIdx];

            Slice targetSl = {-1,-1};
            if (matchRuleToken.splitedValue[TOKEN_ASM_UOP_IDENT][0] == TOKEN_ASM_TYPE_UOP_IDX){
                token instrSliceToken = slaveTokenMap[matchRuleToken.splitedValue[TOKEN_ASM_UOP_IDENT]];
                targetSl = instrSliceToken.sl;
            }else if (matchRuleToken.splitedValue[TOKEN_ASM_UOP_IDENT][0] == TOKEN_ASM_TYPE_UOP_DIR_IDX){
                targetSl = {std::stoi(matchRuleToken.splitedValue[TOKEN_ASM_UOP_DIRECT_START_IDX]),
                                 std::stoi(matchRuleToken.splitedValue[TOKEN_ASM_UOP_DIRECT_STOP_IDX])};
            }
            assert(targetSl.checkValidSlice());
            assert(targetSl.stop <= instr->getOperableSlice().getSize());


            Operable* slicedInstr = instr->doSlice(targetSl);

            assert(matchRuleToken.sl == Slice());
            assert(matchValueToken.sl == Slice());

            std::string rawMatchValue = matchValueToken.splitedValue[TOKEN_ASM_UOP_IDENT];
            ull         matchValue   = std::stoi(rawMatchValue, nullptr, 2);

            activateConditions.push_back( &((*slicedInstr) == matchValue));
        }

        assert(!activateConditions.empty());
        return joinOpr(activateConditions, BITWISE_AND);
    }

    ////////////////////////////
    /// MasterRule
    ////////////////////////////
    ///

    MasterRule::MasterRule
    (InstrRepo* repo, std::string mopName,std::string rule):
    _master(repo),
    _mopName(std::move(mopName)),
    _masterTokens(std::move(rule), true),
    _immAsm(_master){

        ///////////////// inteprt The token
        for (const token& tk: _masterTokens._tokens){
            std::vector<std::string> strVecTk = tk.splitedValue;
            assert(!strVecTk.empty());
            /////case OP
            if (strVecTk.size() == 1){
                /////// it is op
                _opcodeTokens.push_back(tk); ////// the main op decoder <0100011>
                continue;
            }
            //// case REG
            if(strVecTk[TOKEN_DEC_TYPE_IDX][0] == TOKEN_ASM_TYPE_REG_IDX){
                RegIdxAsm worker(_master,tk);
                if (worker.isRead){_srcRegAsms.push_back(worker);}
                else              {_desRegAsms.push_back(worker);}
                continue;
            }
            //// case IMM
            if(strVecTk[TOKEN_DEC_TYPE_IDX][0] == TOKEN_ASM_TYPE_IMM_IDX){
                _immAsm.addImmMeta(tk);
                continue;
            }
            //// case UOP
            if(strVecTk[TOKEN_DEC_TYPE_IDX][0] == TOKEN_ASM_TYPE_UOP_IDX){
                ///// u1
                std::string uopIdent = strVecTk[TOKEN_DEC_TYPE_IDX];
                //////////                    v------------ ujanway   -> token
                _slaveTokenMap.insert({uopIdent, tk});
            }
        }
        ////// flatten the mop
        flattenMop();

    }

    void MasterRule::flattenMop(){
        _flattenOp = genConString('0', _master->getInstrSize());
        for(token& tk: _opcodeTokens){
            int actualStrStart = _master->getInstrSize() - tk.sl.stop;
            int actualStrStop  = _master->getInstrSize() - tk.sl.start;
            assert((actualStrStop-actualStrStart) == tk.sl.getSize());
            _flattenOp.replace(
                actualStrStart,
                actualStrStop-actualStrStart,
                tk.splitedValue[0]
            );
        }
    }

    MasterRule& MasterRule::ad(const std::string& slaveRule,
                                        const std::vector<MatchSlaveValInput>&
                                        slaveMatch){
        for(const auto& match : slaveMatch){
            ////// check uop first is match uop

            assert(_master->isThereUopDec(_mopName, match.effUop));
            _slaveRules.emplace_back(slaveRule, match.matchVal, match.effUop);
        }
        return *this;
    }

    MasterRule& MasterRule::adm(const std::vector<std::string>& uopsName){
        for (const auto& uopName: uopsName){
            _dummyUopSetter.insert(uopName);
        }
        return *this;
    }


    void MasterRule::doAsm(){

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
        _immAsm.doAsm();
        if (_immAsm._regCnt != -1){
            effSrc[_immAsm._regCnt] = true;
        }

        /** disable unused hw*/
        unsetUnusedReg(effSrc, _master->getAmtSrcReg(), true);
        unsetUnusedReg(effDes, _master->getAmtDesReg(), false);

        /* set uop hardware*/
        doUopAsm();


    }

    void MasterRule::doUopAsm(){

        /////// get mop hardware
        OP_HW& opHw = _master->getOp(_mopName);
        int mopIdx = opHw._mopIdx;

        for(const auto&[uopName,uopIdx]: opHw.uopMapIdx){
            //////////////// check dummy first
            if (_dummyUopSetter.find(uopName) != _dummyUopSetter.end()){
                opHw.setUop(uopName, &makeOprVal("setDummyStart",1,1));
                continue;
            }
            //////////////// search all uop
            std::vector<Operable*> activateCons; ///// the activation condition for
            for(auto slaveRule:  _slaveRules){
                if (slaveRule._effUopName == uopName){
                    activateCons.push_back(
                        slaveRule.getActCond(_slaveTokenMap, _master->getInstrOpr()));
                }
            }
            if (activateCons.empty()){
                opHw.setUop(uopName, &makeOprVal("setDummy",1,0));
            }else{
                opHw.setUop(uopName, joinOpr(activateCons, BITWISE_OR));
            }
        }

        ///////// unset all backward mopname

        for (int i =0; i < _master->getAmtMop(); i++){
            if (i != mopIdx){
                _master->getOp(i).reset();
            }else{
                _master->getOp(i).set();
            }
        }


    }

    void MasterRule::unsetUnusedReg(const bool* eff, int size, bool isSrc){
        for (int regIdx = 0; regIdx < size; regIdx++){
            OPR_HW& regHw = isSrc ? _master->getSrcReg(regIdx) : _master->getDesReg(regIdx);
            if (!eff[regIdx]){
                regHw.reset();
            }
        }
    }
}