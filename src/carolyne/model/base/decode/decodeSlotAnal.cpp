//
// Created by tanawin on 13/2/2025.
//

#include "decodeSlotAnal.h"


namespace kathryn::carolyne{


    DecodeSlotAnalyzer::DecodeSlotAnalyzer(IsaBaseRepo& isaBaseRepo):
    _isaBaseRepo(isaBaseRepo){
        mfAssert(!isaBaseRepo.getUopTypes().empty(), "no uop to decode");
    }

    void DecodeSlotAnalyzer::addOprsInConcern(UopTypeBase* uopType, bool isSrc){
        std::vector<OprTypeBase*>& relatedPooledOprs
        = isSrc ? pooledSrcOprs : pooledDesOprs;

        std::vector<OprTypeBase*> relatedUopOprs
        = uopType->getOprTypes(isSrc);

        for(int addIdx = 0; addIdx < relatedUopOprs.size(); addIdx++){
            int findedIdxInpool = findIdxInPool(uopType, addIdx, isSrc);
            if (findedIdxInpool == UNDEF_OPR_IN_POOL){
                OprTypeBase* newAddedOprType = relatedUopOprs[addIdx];
                relatedPooledOprs.push_back(newAddedOprType);
            }
        }

    }

    void DecodeSlotAnalyzer::addUopInConcern(UopTypeBase* uopType){
        crlAss(uopType != nullptr, "cannot add nullptr uop");
        addOprsInConcern(uopType, true);
        addOprsInConcern(uopType, false);
    }

    int DecodeSlotAnalyzer::findIdxInPool(UopTypeBase* uopType,
                                     int oprIdxInUop, bool isSrc){
        crlAss(uopType != nullptr, "uop to anal cannot be nullptr");
        int recurTimes = uopType->findUopRecurTime(oprIdxInUop, isSrc);

        std::vector<OprTypeBase*>& relatedPoolOprs = isSrc ? pooledSrcOprs: pooledDesOprs;
        int matchCount = 0;
        int iterIdx = 0;

        while (iterIdx < relatedPoolOprs.size()){

            if (uopType->getOprTypes(isSrc)[oprIdxInUop] == relatedPoolOprs[iterIdx]){
                matchCount++;
            }

            if (matchCount == recurTimes){
                return iterIdx;
            }
            iterIdx++;
        }

        return UNDEF_OPR_IN_POOL; ///// can't find
    }


    /////////////////// command function

    void DecodeSlotAnalyzer::analIsa(){
        _maxIdentWidth = _isaBaseRepo.getUopTypes()[0]->_fopIdentWidth;
        for (UopTypeBase* uopType: _isaBaseRepo){
            addUopInConcern(uopType);
        }
    }

    void DecodeSlotAnalyzer::addOprsTORowMeta(RowMeta& desRowMeta, bool isSrc){
        std::vector<OprTypeBase*>& pooledRelatedOprs = isSrc ? pooledSrcOprs: pooledDesOprs;
        for(int idx = 0; idx < pooledSrcOprs.size(); idx++){
            ////// [valid, opr_raw_src]
            desRowMeta.addField(getValidFieldName(idx, isSrc), 1);
            desRowMeta.addField(getDecOprFieldName(idx, isSrc),
                                pooledRelatedOprs[0]->getOprWidth());
        }

    }
    void DecodeSlotAnalyzer::buildDecoderSlot(){

        crlAss(_maxIdentWidth > 0, "cannot have buildDecoder Slot with uopWidth " +std::to_string(_maxIdentWidth));
        RowMeta decodeRowMeta;
        decodeRowMeta.addField(SLOT_F_DEC_UOP, _maxIdentWidth);
        addOprsTORowMeta(decodeRowMeta, true);
        addOprsTORowMeta(decodeRowMeta, false);

        _regSlot = new RegSlot(decodeRowMeta, -1);

    }

    RegSlot& DecodeSlotAnalyzer::getDecoderSlot() const{
        crlAss(_regSlot != nullptr, "you may forget to buildDecoderSlot");
        return *_regSlot;
    }

    void DecodeSlotAnalyzer::blk_decode(Slot& rawSlot){

        for(MopTypeBase* mopType: _isaBaseRepo.getMopTypes()){
            crlAss(!mopType->getMatchedSlices().empty(), "mop must have identifier");
            crlAss(!mopType->getUopTypes().empty(), "mop must have at least one uop");
            ////// get raw operable
            Operable* rawData   = rawSlot.getHwSlotMeta(SLOT_F_RAWDATA).opr;
            ////// get Slice to decode region
            Slice     IdentSl   = *mopType->getMatchedSlices().begin();
            Operable* slicedMopIdent = rawData->doSlice(IdentSl);
            ////// checking value
            Operable& checkMopMatch = (*slicedMopIdent) == mopType->getMopIdentValue();

            //////// decode !
            zif(checkMopMatch){ decodeUop(rawSlot, mopType->getUopTypes()[0], mopType->getUopMatcher()[0]); }

                /////// TODO for now we have only 1 uop

        }

    }

    void DecodeSlotAnalyzer::decodeOprs (Slot& rawSlot, UopTypeBase* uopType, UopMatcherBase* uopMatcher, bool isSrc){
        auto relatedOprMatchers = uopMatcher->getOprMatcher(isSrc);
        auto relatedOprs = uopType->getOprTypes(isSrc);
        crlAss(relatedOprs.size() == relatedOprMatchers.size(), "cannot decode opr with mismatch size");

        ///// for track which opr is not decoded
        bool decodedOprs[MAX_DECODE_OPR_NUMBER];
        std::fill(decodedOprs, decodedOprs + MAX_DECODE_OPR_NUMBER, false);

        for (int idx = 0; idx < relatedOprs.size(); idx++){
            /////// for raw
            OprMatcherBase* oprMatch = relatedOprMatchers[idx];
            Slice oprMatchSlice = oprMatch->getMatchedSlices()[0]; ///// TODO we will support diverse decode section later
            Operable* rawData   = rawSlot.getHwSlotMeta(SLOT_F_RAWDATA).opr;
            Operable* slicedRaw = rawData->doSlice(oprMatchSlice);
            /////// des regSlot
            RegSlot* decodedSlot = _regSlot;
            int idxInpooled = findIdxInPool(uopType, idx, isSrc);
            std::string validFieldName = getValidFieldName(idx, isSrc);
            std::string oprFieldName   = getDecOprFieldName(idx, isSrc);

            decodedSlot.assignCore(validFieldName, validSetter);
            decodedSlot.assignCore(oprFieldName, *slicedRaw);
            decodedOprs[idxInpooled] = true;
        }

        ////// de valid the operand that don't have related in the decode
        auto& relatedPooledOprs = isSrc ? pooledSrcOprs: pooledDesOprs;
        for (int idx = 0; idx < relatedPooledOprs.size(); idx++){
            if (decodedOprs[idx]){
                continue;
            }
            RegSlot* decodedSlot = _regSlot;
            std::string validFieldName = getValidFieldName(idx, isSrc);
            decodedSlot.assignCore(validFieldName, validUnset);
        }

    }

    ////////// aim to map rawSlot to slot decoder
    void DecodeSlotAnalyzer::decodeUop(Slot& rawSlot,
                                       UopTypeBase* uopType,
                                       UopMatcherBase* uopMatcher){
        ///////// uop ident Assign
        Val& UOP_IDENT = makeOprVal("UOP_IDENT_" + uopType->_uopName, _maxIdentWidth, uopType->getIdentVal());
        _regSlot->assignCore(SLOT_F_DEC_UOP, UOP_IDENT);
        ///////// decode operand
        decodeOprs(rawSlot, uopType, uopMatcher, true);
        decodeOprs(rawSlot, uopType, uopMatcher, false);
    }


    ////////// decoder slot

    std::string DecodeSlotAnalyzer::getValidFieldName(int idx, bool isSrc){

        std::vector<OprTypeBase*>& pooledRelatedOprs = isSrc ? pooledSrcOprs: pooledDesOprs;
        crlAss(idx < pooledRelatedOprs.size(),
            "get Valid fieled with idx " + std::to_string(idx));
        return std::string(isSrc ? SLOT_F_DEC_OPR_VALID_SRC : SLOT_F_DEC_OPR_VALID_DES)
                + "_" + std::to_string(idx);
    }

    std::string DecodeSlotAnalyzer::getDecOprFieldName(int idx, bool isSrc){
        std::vector<OprTypeBase*>& pooledRelatedOprs = isSrc ? pooledSrcOprs: pooledDesOprs;
        crlAss(idx < pooledRelatedOprs.size(),
            "get Valid fieled with idx " + std::to_string(idx));
        return std::string(isSrc ? SLOT_F_DEC_OPR_RAW_SRC : SLOT_F_DEC_OPR_RAW_DES)
                + "_" + std::to_string(idx);
    }







}
