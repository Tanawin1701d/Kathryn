//
// Created by tanawin on 9/7/2024.
//

#include "instrBase.h"

#include <utility>
#include "model/flowBlock/cond/zelif.h"
#include "model/flowBlock/cond/zif.h"
#include "util/numberic/pmath.h"
#include "util/termColor/termColor.h"
#include "util/str/strUtil.h"


namespace kathryn{

////////////////////////// OPR HW

    OPR_HW::OPR_HW(int archSize, int idxSize, int regNo, bool isSrc):
    _isSrc(isSrc),
    data (mOprReg("regData"  + std::to_string(regNo) + (isSrc ? "Src": "Des"), archSize)),
    idx  (mOprReg("regIdx"   + std::to_string(regNo) + (isSrc ? "Src": "Des"), idxSize)),
    valid(mOprReg("regValid" + std::to_string(regNo) + (isSrc ? "Src": "Des"), 1)){
        assert( (1 << idxSize) >= archSize);
        assert(regNo >= 0);
    }

    void OPR_HW::reset(){
        valid <<= 1;
        idx   <<= 0;
    }

    void OPR_HW::setOnlyIndex(Operable* index){
        assert(index != nullptr);
        assert(index->getOperableSlice().getSize() == idx.getOperableSlice().getSize());
        valid <<= 0;
        idx   <<= *index;
    }

    void OPR_HW::setImm(Operable* opr){
        assert(opr != nullptr);
        assert(opr->getOperableSlice().getSize() == data.getOperableSlice().getSize());
        valid <<= 1;
        idx   <<= 0;
        data  <<= *opr;
    }

    void OP_HW::set(){
        _set <<= 1;
    }

    void OP_HW::reset(){
        _set <<= 0;
    }

    void OP_HW::setUop(int idx, Operable* condition){
        assert(condition != nullptr);
        assert(condition->getOperableSlice().getSize() == 1);
        (*_uopSets.at(idx)) <<= (*condition);
    }

////////////////////////// OP HW
    OP_HW::OP_HW(int mopIdx,
                 const std::string& mopName,
                 std::vector<UopAsm>& uops):
    _mopIdx(mopIdx),
    _mopName(mopName),
    _set(mOprReg("mop_" + mopName, 1)){
        assert(mopIdx >= 0);
        for(UopAsm& uop: uops){
            _uopSets.push_back(&mOprReg("uop_" + uop._uopName, 1));
        }
    }

///////////////////////// RULE

    RULE::RULE(int mopIdx, std::string  name, std::string  rule):
    _mopIdx(mopIdx),
    _name  (std::move(name)),
    _rule  (std::move(rule)){
        assert(_mopIdx >= 0);
    }

    void RULE::startTokeniz(){
        std::stack<token> tokenSt;
        /////// iterate all in string
        for (int idx = 0; idx < _rule.size(); idx++){
            char bitChar = _rule[idx];
            switch (bitChar){
                case '<':{tokenSt.emplace(); break;}
                case '>':{
                    token topToken = tokenSt.top();
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

        int nextStartIdx = 0;
        for (auto tk = _tokens.rbegin(); tk != _tokens.rend(); tk++){
            tk->finalToken(nextStartIdx);
            nextStartIdx += tk->sl.getSize();
        }
        assert(tokenSt.empty());
    }
/////////////////////////

    std::pair<
            std::vector<MOP*>,
            std::vector<MOP*>
    >
    InstrRepo::seperateMopByopcode(std::vector<MOP*>& grpMop, int bitIdx){
        std::vector<MOP*> mopWithZero;
        std::vector<MOP*> mopWithOne;

        for (MOP* mop: grpMop){
            assert(mop != nullptr);
            char decVal = mop->getFlattenUop()[bitIdx];
            switch(decVal){
            case '0' : {mopWithZero.push_back(mop); break;}
            case '1' : {mopWithOne .push_back(mop); break;}
            default : {assert(false);}
            }
        }
        return {mopWithZero, mopWithOne};
    }

    void InstrRepo::genDecInternal(std::vector<MOP*>& grpMop, int bitIdx){
        if (grpMop.empty()){return;}

        if (bitIdx == -1){
            assert(grpMop.size() == 1); ///// there must be only one
            grpMop[0]->doAllAsm();
            return;
        }

        auto [zeros, ones] =
            seperateMopByopcode(grpMop, bitIdx);

        bool zeroEmpty = zeros.empty();
        bool oneEmpty  = ones .empty();

        if ((!zeroEmpty) && (!oneEmpty)){
            ////// seperate into two section if get condition
            ////// and not condition
            int archBit = INSTR_WIDTH - 1 - bitIdx;
            zif(
                *( (*_instr).doSlice({archBit, archBit+1}) )
            ){
                genDecInternal(ones, bitIdx-1);
            }zelse{
                genDecInternal(zeros, bitIdx-1);
            }
        }else{
            /////// the bit cannot be classified
            /// next find the other bit
            genDecInternal(ones , bitIdx-1);
            genDecInternal(zeros, bitIdx-1);
        }

    }

    InstrRepo::InstrRepo(int instrWidth, int amtSrcOpr,
                         int amtDesOpr , int oprWidth,
                         Operable* instr):
    _instr     (instr     ),
    INSTR_WIDTH(instrWidth),
    OPR_WIDTH  (oprWidth  ),
    _amtMopType(0),
    _amtSrcOpr (amtSrcOpr),
    _amtDesOpr (amtDesOpr){
        assert(instr != nullptr);
        assert(INSTR_WIDTH  > 0);
        assert(OPR_WIDTH    > 0);
        assert(amtSrcOpr   >= 0);
        assert(amtDesOpr   >= 0);
    }

    InstrRepo::~InstrRepo(){
        for(OPR_HW* oprHw: srcOprs){
            delete oprHw;
        }
        for(OPR_HW* oprHw: desOprs){
            delete oprHw;
        }
        for(OP_HW*   opHw: opcodes){
            delete opHw;
        }

    }


    void InstrRepo::addMop(const std::string& rule,
                           const std::string& ruleName){
        assert(!rule.empty());
        mopRules.emplace_back(_amtMopType, ruleName, rule);
        _amtMopType++;
    }

    void InstrRepo::addUop(std::vector<UOP_INPUT_META> uopMetas){
        for (UOP_INPUT_META& uopMeta: uopMetas){
            assert(!uopMeta.rule.empty());
            assert(uopMeta.mopIdx < _amtMopType);
            uopRules.emplace_back(uopMeta.mopIdx, uopMeta.ruleName, uopMeta.rule);
        }
    }

    void InstrRepo::processToken(){
        for (RULE& rule: mopRules){
            rule.startTokeniz();
        }
        for (RULE& rule: uopRules){
            rule.startTokeniz();
        }

        /////// init mop
        for(int i = 0; i < mopRules.size(); i++){
            assert(i == mopRules[i]._mopIdx);
            mops.emplace_back(this, mopRules[i]._tokens, mopRules[i]._name, i);
            mops[i].interpretMasterToken();
        }

        for (int uopIdx = 0;  uopIdx < uopRules.size(); uopIdx++){
            RULE& uopRule = uopRules[uopIdx];
            assert(uopRule._mopIdx < _amtMopType);
            mops[uopRule._mopIdx].createUop(uopRule._tokens, uopRule._name);
        }

        for(int i = 0; i < mopRules.size(); i++){
            mops[i].flattenMop(); ///// generate data
        }
    }

    void InstrRepo::declareHw(){
        //////// declareOperand
        for (int i = 0; i < _amtSrcOpr; i++){
            srcOprs.push_back(new OPR_HW(
                OPR_WIDTH,
                log2Ceil(OPR_WIDTH),
                i, true));
        }
        for (int i = 0; i < _amtDesOpr; i++){
            desOprs.push_back(new OPR_HW(
                OPR_WIDTH,
                log2Ceil(OPR_WIDTH),
                i, false));
        }

        for (int i = 0; i < mops.size(); i++){
            opcodes.push_back(new OP_HW(i, mops[i]._mopName,mops[i]._uops));
        }
    }

    void InstrRepo::genDecodeLogic(){
        /////// recruit all uop
        std::vector<MOP*> poolMop;
        for (MOP& mop: mops){
            poolMop.push_back(&mop);
        }
        /////// generate all uop
        genDecInternal(poolMop, INSTR_WIDTH-1);
    }

    OPR_HW& InstrRepo::getSrcReg(int idx){
        assert(idx < _amtSrcOpr);
        return *srcOprs[idx];
    }
    OPR_HW& InstrRepo::getDesReg(int idx){
        assert(idx < _amtDesOpr);
        return *desOprs[idx];
    }
    OP_HW&  InstrRepo::getOp    (int mopIdx){
        assert(mopIdx < _amtMopType);
        return *opcodes[mopIdx];
    }
}
