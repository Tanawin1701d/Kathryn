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

    OPR_HW::OPR_HW(int archSize, int idxSize, int regNo):
    data(makeOprReg("regData" + std::to_string(regNo), archSize)),
    idx(makeOprReg("regIdx" + std::to_string(regNo),idxSize)),
    valid(makeOprReg("regValid" + std::to_string(regNo), 1)){
        assert(1 << idxSize >= archSize);
        assert(regNo > 0);
    }

////////////////////////// OP HW
    OP_HW::OP_HW(int typeId, const std::string& typeName,
                 std::vector<UopAsm>& uops):
    _typeIdx(typeId),
    _typeName(typeName),
    _set(makeOprReg("mop_" + typeName, 1))
    {
        assert(typeId >= 0);
        for(UopAsm& uop: uops){
            _uopSets.push_back(&makeOprReg("uop_" + uop._uopName, 1));
        }
    }

///////////////////////// RULE

    RULE::RULE(int typeIdx, int uopIdx,
        std::string  name, std::string  rule):
    _typeIdx(typeIdx),
    _uopIdx(uopIdx),
    _name(std::move(name)),
    _rule(std::move(rule)){}

    void RULE::startTokeniz(int instrBitSize){
        assert(instrBitSize > 0);
        int curIdx = instrBitSize;
        std::stack<token> tokenSt;
        /////// iterate all in string
        for (int idx = 0; idx < _rule.size(); idx--){
            char bitChar = _rule[idx];
            switch (bitChar){
            case '<':{
                tokenSt.push({{-1, curIdx}});
                break;
            }
            case '>':{
                token topToken = tokenSt.top();
                topToken.finalToken();
                _tokens.push_back(topToken);
                tokenSt.pop();
                curIdx -= topToken.sl.getSize();
                break;
            }
            default :{
                assert(!tokenSt.empty());
                tokenSt.top().value += bitChar; break;
            }
            }
        }
        if (curIdx != 0){
            std::cout << "[warning] instruction tokenizing not complete" << std::endl;
        }
        assert(tokenSt.empty());
    }

    UopAsm RULE::genUopAsm(InstrRepo* repo) const{
        assert(repo != nullptr);
        assert(_typeIdx > 0);
        assert(_uopIdx > 0);
        return {repo, _tokens, _name, _typeIdx, _uopIdx};
    }

    bool MOP::checkValidMop() const{ return (_typeIdx >= 0) && (!_uops.empty()); }

/////////////////////////

    std::pair<
            std::vector<UopAsm*>,
            std::vector<UopAsm*>
    >
    InstrRepo::seperateUopByopcode(std::vector<UopAsm*>& uops, int bitIdx){
        std::vector<UopAsm*> uopWithZero;
        std::vector<UopAsm*> uopWithOne;

        for (UopAsm* uop: uops){
            assert(uop != nullptr);
            char decVal = uop->getFlattenOpcode()[bitIdx];
            switch(decVal){
            case '1' : {uopWithOne .push_back(uop); break;}
            case '0' : {uopWithZero.push_back(uop); break;}
            default : {assert(false);}
            }
        }
        return {uopWithZero, uopWithOne};
    }

    void InstrRepo::genDecInternal(std::vector<UopAsm*>& uops, int bitIdx){
        if (uops.empty()){
            return;
        }

        if (bitIdx == -1){
            assert(uops.size() == 1); ///// there must be only one
            uops[0]->doAllAsm();
            return;
        }

        auto [zeros, ones] =
            seperateUopByopcode(uops, bitIdx);

        bool zeroEmpty = zeros.empty();
        bool oneEmpty  = ones.empty();

        if ( (!zeroEmpty) && (!oneEmpty) ){
            int archBit = INSTR_SIZE - 1 - bitIdx;
            zif(
                *( (*_instr).doSlice({archBit, archBit+1}) )
            ){
                genDecInternal(ones, bitIdx-1);
            }zelse{
                genDecInternal(zeros, bitIdx-1);
            }
        }else{
            genDecInternal(ones , bitIdx-1);
            genDecInternal(zeros, bitIdx-1);
        }

    }

    InstrRepo::InstrRepo(int instrSize, int amtInstrType,
                         int amtSrcOpr, int amtDesOpr,
                         Operable* instr):
    _instr(instr),
    INSTR_SIZE(instrSize),
    _amtInstrType(amtInstrType),
    _amtSrcOpr(amtSrcOpr),
    _amtDesOpr(amtDesOpr){
        mops.resize(amtInstrType);
        assert(instr != nullptr);
        assert(instrSize    > 0);
        assert(amtInstrType > 0);
        assert(amtSrcOpr    >= 0);
        assert(amtDesOpr    >= 0);
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


    void InstrRepo::addRule(const std::string& rule,
                            const std::string& ruleName,
                            int typeIdx,int uopIdx){
        assert(!rule.empty());
        rules.emplace_back(typeIdx, uopIdx, ruleName, rule);
    }

    void InstrRepo::convertRuleToMop(){

        for(RULE& rule: rules){
            ///// tokenize and build the uop
            rule.startTokeniz(INSTR_SIZE);
            UopAsm uopWorker = rule.genUopAsm(this);
            uopWorker.assignMasterToAsm();
            uopWorker.startGetSumOpcode();
            ///// mops start mop
            assert(uopWorker._typeIdx < mops.size());
            mops[uopWorker._typeIdx].addUopAsm(uopWorker);
        }
        ////isntruction checker
        for(int idx = 0; idx < _amtInstrType; idx++){
            if (!mops[idx].checkValidMop()){
                std::cout << TC_YELLOW << "[warning] mop " << idx <<
                    " is not initialize" << TC_DEF << std::endl;
            }
        }

    }

    void InstrRepo::declareHw(){
        //////// declareOperand
        for (int i = 0; i < _amtSrcOpr; i++){
            srcOprs.push_back(new OPR_HW(
                INSTR_SIZE,
                log2Ceil(INSTR_SIZE),
                i));
        }
        for (int i = 0; i < _amtDesOpr; i++){
            desOprs.push_back(new OPR_HW(
                INSTR_SIZE,
                log2Ceil(INSTR_SIZE),
                i));
        }

        for (int i = 0; i < mops.size(); i++){
            opcodes.push_back(new OP_HW(i, mops[i].mopName,mops[i]._uops));
        }
    }

    void InstrRepo::genDecodeLogic(){
        /////// recruit all uop
        std::vector<UopAsm*> poolUop;
        for (MOP& mop: mops){
            for(UopAsm& uop: mop._uops){
                poolUop.push_back(&uop);
            }
        }
        /////// generate all uop
        genDecInternal(poolUop, INSTR_SIZE-1);
    }

    OPR_HW& InstrRepo::getSrcReg(int idx){
        assert(idx < _amtSrcOpr);
        return *srcOprs[idx];
    }
    OPR_HW& InstrRepo::getDesReg(int idx){
        assert(idx < _amtDesOpr);
        return *desOprs[idx];
    }
    OP_HW&  InstrRepo::getOp    (int typeId){
        assert(typeId < _amtInstrType);
        return *opcodes[typeId];
    }

}
