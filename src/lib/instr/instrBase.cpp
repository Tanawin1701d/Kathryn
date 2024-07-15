//
// Created by tanawin on 9/7/2024.
//

#include "instrBase.h"

#include <set>
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



////////////////////////// OP HW
    OP_HW::OP_HW(int mopIdx,
                 const std::string& mopName,
                 std::vector<std::string>& uopNames):
    _mopIdx(mopIdx),
    _mopName(mopName),
    _set(mOprReg("mop_" + mopName, 1)){
        assert(mopIdx >= 0);
        for(const std::string& uopName: uopNames){
            int uopIdx = _uopSets.size();
            _uopSets.push_back(&mOprReg("uop_" + uopName, 1));
            assert(uopMapIdx.find(uopName)==uopMapIdx.end());
            uopMapIdx.insert({uopName, uopIdx});
        }
    }

    void OP_HW::set(){ _set <<= 1;}

    void OP_HW::reset(){_set <<= 0;}

    void OP_HW::setUop(const std::string& uopName, Operable* condition){
        assert(condition != nullptr);
        assert(condition->getOperableSlice().getSize() == 1);
        assert(uopMapIdx.find(uopName) != uopMapIdx.end());
        int idx = uopMapIdx[uopName];
        (*_uopSets.at(idx)) <<= (*condition);
    }

    Reg& OP_HW::isSet(){ return _set;}

    Reg& OP_HW::isUopSet(const std::string& uopName){
        assert(uopMapIdx.find(uopName) != uopMapIdx.end());
        int uopIdx = uopMapIdx[uopName];
        assert(uopIdx < _uopSets.size());
        assert(_uopSets[uopIdx] != nullptr);
        return *_uopSets[uopIdx];
    }

////////////////////
/////// InstrRepo
////////////////////


bool MOP_META::checkValid() const{

        std::set<std::string> x;

        for (const auto& uopName: uopNames){
            if(x.find(uopName) != x.end()){
                return false;
            }
            x.insert(uopName);
        }
        return true;

}


    std::pair<
            std::vector<MasterRule*>,
            std::vector<MasterRule*>
    >
    InstrRepo::seperateMopByopcode(std::vector<MasterRule*>& grpMop, int bitIdx){
        std::vector<MasterRule*> mopWithZero;
        std::vector<MasterRule*> mopWithOne;

        for (MasterRule* mop: grpMop){
            assert(mop != nullptr);
            char decVal = mop->getFlattenOp()[bitIdx];
            switch(decVal){
            case '0' : {mopWithZero.push_back(mop); break;}
            case '1' : {mopWithOne .push_back(mop); break;}
            default : {assert(false);}
            }
        }
        return {mopWithZero, mopWithOne};
    }

    void InstrRepo::genDecInternal(std::vector<MasterRule*>& grpMop, int bitIdx){
        if (grpMop.empty()){return;}

        if (bitIdx == -1){
            assert(grpMop.size() == 1); ///// there must be only one
            grpMop[0]->doAsm();
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

void InstrRepo::addMop(const MOP_META& mopMeta){
        assert(mopMeta.checkValid());
        assert(opcodeMap.find(mopMeta.mopName) == opcodeMap.end());
        opcodeMap.insert({mopMeta.mopName, mopMetas.size()});
        mopMetas.push_back(mopMeta);
}

void InstrRepo::addDecRule(const std::string& workOnMopName,
                           const std::string& rule){
        masterRules.emplace_back(this, workOnMopName, rule);
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

        for (int i = 0; i < mopMetas.size(); i++){
            opcodes.push_back(
                new OP_HW(i,mopMetas[i].mopName,
                    mopMetas[i].uopNames));
        }
}


void InstrRepo::genDecodeLogic(){

        std::vector<MasterRule*> masterRulePool;
        for (MasterRule& msr: masterRules){
            masterRulePool.push_back(&msr);
        }
        genDecInternal(masterRulePool, INSTR_WIDTH-1);

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
OP_HW&  InstrRepo::getOp    (const std::string& mopName){
    assert(opcodeMap.find(mopName) != opcodeMap.end());
    return getOp(opcodeMap[mopName]);
}

}
