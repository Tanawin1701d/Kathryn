//
// Created by tanawin on 17/1/2025.
//


#include <utility>

#include "rowMeta.h"
#include "slot.h"
#include "model/controller/controller.h"

#ifndef src_lib_hw_slot_TABLE_H
#define src_lib_hw_slot_TABLE_H


namespace kathryn{

    struct Candidate{
        SlotOpr   detLogic; //// determine logic
        Operable* detIdx = nullptr;
    };

    class Table{
    public:
        std::string        _tableName = "UNNAME_TABLE";
        RowMeta            _meta;
        const int          _identWidth = -1;
        const int          _amtSize    = -1;
        std::vector<Slot*> _hwSlots; //// for collect hardware


    Table(RowMeta  meta, std::string tableName, int identWidth = 0):
        _tableName (std::move(tableName)),
        _meta      (std::move(meta)),
        _identWidth(identWidth),
        _amtSize   (1 << identWidth){

        mfAssert(_identWidth >= 0, "table ident ");
        std::cout << "generate " << _amtSize << " slots" << std::endl;
        for (int i = 0; i < _amtSize; i++){
            Slot* slotPtr = &mOprMod(_tableName + "_slot_" + std::to_string(i),Slot,_meta, i);
            _hwSlots.push_back(slotPtr);
        }
    }

    Table(const std::vector<std::string>& fieldNames,
          const std::vector<int>&         fieldSizes,
          std::string tableName, int identWidth = 0):
        _tableName (std::move(tableName)),
        _meta      (fieldNames, fieldSizes),
        _identWidth(identWidth),
        _amtSize   (1 << identWidth){

        mfAssert(_identWidth >= 0, "table ident ");
        std::cout << "generate " << _amtSize << " slots" << std::endl;
        for (int i = 0; i < _amtSize; i++){
            Slot* slotPtr = &mOprMod(_tableName + "_slot_" + std::to_string(i),Slot,_meta, i);
            _hwSlots.push_back(slotPtr);
        }
    }


    SlotOpr buildSelectLogic(Operable& selectLhs,
                             SlotOpr lhsSlotOpr,
                             SlotOpr rhsSlotOpr,
                             const std::string& prefixName){ //// for debug purpose only

        std::vector<Operable*> selectedOperable;
        for(const FieldMeta& fieldMeta: lhsSlotOpr.getRowMeta().getAllFields()){
            ///// build new selected wire for each wire in each field
            Wire& joinWire = mOprWire(prefixName + "_"+ "selected" + fieldMeta._fieldName, fieldMeta._fieldSize);
            joinWire.addUpdateMeta(
                new UpdateEvent{
                    &selectLhs ,
                    nullptr,
                    &lhsSlotOpr.getOpr(fieldMeta._fieldName),
                    lhsSlotOpr.getOpr(fieldMeta._fieldName).getOperableSlice(),
                    DEFAULT_UE_PRI_USER});
            joinWire.addUpdateMeta(
                new UpdateEvent{
                    &(~selectLhs),
                    nullptr,
                    &rhsSlotOpr.getOpr(fieldMeta._fieldName),
                    rhsSlotOpr.getOpr(fieldMeta._fieldName).getOperableSlice(),
                    DEFAULT_UE_PRI_USER});
            selectedOperable.push_back(&joinWire);
        }
        return SlotOpr(lhsSlotOpr.getRowMeta(), selectedOperable);
    }



    ///// reqIdx used that Output require the index of table or not
    ///  matchCondition must return single bit whether select lhs (true) or rhs (false)

    Candidate buildCmpSearchLogic(const RowMeta& rowMeta,        //////// the field that designer concern
                                  bool  reqSlotIdx,              //////// request build idx for each comparison
                                  const std::function<Operable&(Operable* lhsIdx,
                                                                SlotOpr   lhsSlotOpr,
                                                                Operable* rhsIdx,
                                                                SlotOpr   rhsSlotOpr)>& cmpCon){

        //////// build representa for each slot row and insert to queue
        std::queue<Candidate> candidates;
        int idx = 0;
        for(Slot* slot: _hwSlots){
            Val* val = nullptr;
            if (reqSlotIdx){
                val = &makeOprVal(_tableName + "_detIdx_"+std::to_string(idx), _identWidth, idx);
            }
            Candidate x = {slot->getSlotOpr(rowMeta), val};
            candidates.push(x);
            idx++;
        }

        //////// build logic tree

        int gatherSize = _amtSize; /// amount size is power of 2
        int level      = 0;

        while(gatherSize != 1){
            for(int i = 0; i < gatherSize; i+=2){
                auto lhs = candidates.front(); candidates.pop();
                auto rhs = candidates.front(); candidates.pop();
                Operable& selectLhs = cmpCon(lhs.detIdx, lhs.detLogic, rhs.detIdx, rhs.detLogic);
                SlotOpr selected = buildSelectLogic(
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

    Candidate buildMinMaxLogic(int fieldIdx, bool reqIdx, bool isMin){

        ////// we will build binary tree
        FieldMeta field = _meta.getField(fieldIdx);
        std::vector<FieldMeta> rowMeta({field});
        return buildCmpSearchLogic(rowMeta, reqIdx, [&](Operable* lhsIdx,
                                                               SlotOpr   lhsSlotOpr,
                                                               Operable* rhsIdx,
                                                               SlotOpr   rhsSlotOpr) -> Operable&{

            ///std::cout << lhsSlotOpr.getFieldMeta(0)._fieldName << std::endl;

            if (isMin){ return lhsSlotOpr.getOpr(0) <= rhsSlotOpr.getOpr(0);}
            return lhsSlotOpr.getOpr(0) >= rhsSlotOpr.getOpr(0);

        });
    }



    Candidate buildMinMaxLogic(const std::string& fieldName, bool reqIdx, bool isMin){
        int idx =  _meta.getFieldIdx(fieldName);
        return buildMinMaxLogic(idx, reqIdx, isMin);
    }

    /////// designer put the `matchCon` function variable to get slot that match designers' requirement
    SlotOpr buildGetLogic(const std::function<Operable&(int idx, SlotOpr examSlotOpr)>& matchCon){
        std::vector<Wire*> slotReps;
        ///// build representation wire
        for (const FieldMeta& fieldMeta: _meta.getAllFields()){
            Wire* wire = &makeOprWire(_tableName + "_getSlot_" + fieldMeta._fieldName, fieldMeta._fieldSize);
            slotReps.push_back(wire);
        }
        ///// build representation wire
        for (int slotIdx = 0; slotIdx < _amtSize; slotIdx++){
            Operable& matchCond = matchCon(slotIdx, _hwSlots[slotIdx]->getSlotOpr());
            /////// build assign for each field
            for (int fieldIdx = 0; fieldIdx < _meta.getSize(); fieldIdx++){
                slotReps[fieldIdx]->addUpdateMeta(new UpdateEvent{
                    &matchCond,
                    nullptr,
                    &_hwSlots[slotIdx]->get(fieldIdx),
                    slotReps[fieldIdx]->getOperableSlice(),
                    DEFAULT_UE_PRI_USER
                });
            }
        }

        std::vector<Operable*> cvtSlotReps;
        cvtSlotReps.reserve(slotReps.size());
        for (Wire* slotRep: slotReps){cvtSlotReps.push_back(slotRep);}
        return SlotOpr(_meta, cvtSlotReps);
    }

    SlotOpr buildGetLogic(Operable& searchIdx){
        return buildGetLogic(
            [&](int idx, const SlotOpr& examSlotOpr) -> Operable&{
                return searchIdx == idx;
            });
    }

    /////// return single wire that used to store signal
    /// system to store

    Wire& buildSetLogic(SlotOpr& slotOpr, Operable& reqIdx){

        int idxSize = reqIdx.getOperableSlice().getSize();

        mfAssert(idxSize == _identWidth, "setLogic bitwidth is mismatch");

        ///// enable signal will trigger that should be establish
        Wire& enable = mOprWire( _tableName + "_setEnableSignal", 1);

        ///// plug all slot to listen
        for (int idx = 0; idx < _hwSlots.size(); idx++){
            expression& checkIdMatch  = reqIdx == idx;
            FieldMeta   oprStartField = slotOpr.getFieldMeta(0);
            FieldMeta   oprEndField   = slotOpr.getFieldMeta(slotOpr.getAmtField()-1);

            _hwSlots[idx]->assignCore(slotOpr,
                oprStartField._fieldName, oprEndField._fieldName,
                checkIdMatch & enable);
        }
        return enable;
    }

    };

}




#endif //__src_lib_hw_slot_TABLE_H
