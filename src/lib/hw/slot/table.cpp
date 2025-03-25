//
// Created by tanawin on 24/3/2025.
//
#include "table.h"
#include "slotUtil.h"

namespace kathryn{



    /**
     *
     * GET REDUCTION TREE LOGIC
     *
     * */


    ///////// reduction tree compare and search
    Candidate Table::buildCmpSearchLogic(const RowMeta& rowMeta,        //////// the field that designer concern
                                         bool  reqSlotIdx,              //////// request build idx for each comparison
                                         const std::function<Operable&(Operable* lhsIdx,
                                                                       Slot      lhsSlot,
                                                                       Operable* rhsIdx,
                                                                       Slot      rhsSlot)>& cmpCon){

        std::queue<Candidate> candidates;   ////// reduction tree queue

        //////// build representa (Candidate) for each slot row and insert to queue
        int idx = 0;
        for(Slot* slot: _hwSlots){
            Val* val = nullptr;
            if (reqSlotIdx){
                val = &makeOprVal(_tableName + "_detIdx_"+std::to_string(idx), _identWidth, idx);
            }
            Candidate x = {*slot, val};
            candidates.push(x);
            idx++;
        }

        //////// build reduction tree
        int gatherSize = _amtSize; /// amount size is power of 2
        int level      = 0;

        while(gatherSize != 1){
            for(int i = 0; i < gatherSize; i+=2){
                auto lhs = candidates.front(); candidates.pop();
                auto rhs = candidates.front(); candidates.pop();
                Operable& selectLhs = cmpCon(lhs.detIdx, lhs.detLogic, rhs.detIdx, rhs.detLogic);
                Slot selected = buildSlotSelectLogic(
                        selectLhs,
                        lhs.detLogic, rhs.detLogic,
                        _tableName + "_logic_" + std::to_string(level));
                Wire* ident = nullptr;
                if (reqSlotIdx){
                    ident = &makeOprWire(_tableName + "_identLevel" + std::to_string(level) + "_" + std::to_string(i), _identWidth);
                    ident->addUpdateMeta(new UpdateEvent{&  selectLhs ,nullptr, lhs.detIdx, ident->getOperableSlice(),DEFAULT_UE_PRI_USER});
                    ident->addUpdateMeta(new UpdateEvent{&(~selectLhs),nullptr, rhs.detIdx, ident->getOperableSlice(),DEFAULT_UE_PRI_USER});
                }
                candidates.push({selected, ident});
            }
            level++;
            gatherSize/=2;
        }
        assert(candidates.size() == 1);
        return candidates.front();
    }

    Candidate Table::buildMinMaxLogic(int fieldIdx, bool reqIdx, bool isMin){
        ////// we will build binary tree
        FieldMeta field = _meta.getField(fieldIdx);
        std::vector<FieldMeta> rowMeta({field});
        return buildCmpSearchLogic(rowMeta, reqIdx, [&](Operable* lhsIdx,
                                                        Slot      lhsSlot,
                                                        Operable* rhsIdx,
                                                        Slot      rhsSlot) -> Operable&{

            if (isMin){ return (*lhsSlot.getHwSlotMeta(fieldIdx).opr) <= (*rhsSlot.getHwSlotMeta(fieldIdx).opr);}
            return (*lhsSlot.getHwSlotMeta(fieldIdx).opr) >= (*rhsSlot.getHwSlotMeta(fieldIdx).opr);
        });
    }

    Candidate Table::buildMinMaxLogic(const std::string& fieldName, bool reqIdx, bool isMin){
        int idx =  _meta.getFieldIdx(fieldName);
        return buildMinMaxLogic(idx, reqIdx, isMin);
    }


    /**
      *
      * GET LOGIC
      *
      **/

    Slot Table::buildGetLogic(const std::function<Operable&(int idx, Slot examSlot)>& matchCon){
        std::vector<Wire*> slotReps;
        ///// build representation wire
        for (const FieldMeta& fieldMeta: _meta.getAllFields()){
            Wire* wire = &makeOprWire(_tableName + "_getSlot_" + fieldMeta._fieldName, fieldMeta._fieldSize);
            slotReps.push_back(wire);
        }
        ///// assign representation wire
        for (int slotIdx = 0; slotIdx < _amtSize; slotIdx++){
            Operable& matchCond = matchCon(slotIdx, *_hwSlots[slotIdx]);
            /////// build assign for each field
            for (int fieldIdx = 0; fieldIdx < _meta.getAmtField(); fieldIdx++){
                slotReps[fieldIdx]->addUpdateMeta(new UpdateEvent{
                        &matchCond,
                        nullptr,
                        _hwSlots[slotIdx]->getHwSlotMeta(fieldIdx).opr,
                        slotReps[fieldIdx]->getOperableSlice(),
                        DEFAULT_UE_PRI_USER
                });
            }
        }

        std::vector<SlotMeta> slotMetas;
        slotMetas.reserve(slotReps.size());
        for (Wire* slotRep: slotReps){slotMetas.push_back({slotRep, slotRep});}
        return Slot(_meta, slotMetas, -1);
    }

    Slot Table::buildGetLogic(Operable& searchIdx){
        return buildGetLogic(
                [&](int idx, const Slot& examSlotOpr) -> Operable&{
                    return searchIdx == idx;
                });
    }

/////// it is pool conditionl we will fix it when require

//    Wire& Table::buildSetLogic(Slot& slot, Operable& reqIdx){
//
//        int idxSize = reqIdx.getOperableSlice().getSize();
//
//        mfAssert(idxSize == _identWidth, "setLogic bitwidth is mismatch");
//
//        ///// enable signal will trigger that should be assigned
//        /// it will return to user
//        Wire& enable = mOprWire( _tableName + "_setEnableSignal", 1);
//
//        ///// plug all slot to listen
//        for (int idx = 0; idx < _hwSlots.size(); idx++){
//            expression& checkIdMatch  = reqIdx == idx;
//            FieldMeta   oprStartField = slot.getMeta().getField(0);
//            FieldMeta   oprEndField   = slot.getMeta().getField(slot.getAmtField()-1);
//
//            // _hwSlots[idx]->assignCore(slot,
//            //     oprStartField._fieldName, oprEndField._fieldName,
//            //     checkIdMatch & enable);
//        }
//        return enable;
//    }

    /////// set logic
    Table& Table::assign(Slot& slot, Operable& reqIdx, bool isBlockAsm){

        int idxSize = reqIdx.getOperableSlice().getSize();
        mfAssert(idxSize == _identWidth, "setLogic bitwidth is mismatch");

        /////// loop to all slot
        for(int idx = 0; idx < _hwSlots.size(); idx++){
            zif(reqIdx == _hwSlots[idx]->getIdent()){
                _hwSlots[idx]->assignCore(slot, isBlockAsm);
            }
        }

        return *this;
    }

    RegSlot& Table::get(int constIdx){
        mfAssert(constIdx < _amtSize, "retrieve data from table: " + _tableName +
                                      " with idx: " + std::to_string(constIdx) +
                                      " is out of range");
        return *_hwSlots[constIdx];
    }



}