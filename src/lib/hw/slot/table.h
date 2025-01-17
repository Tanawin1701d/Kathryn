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
        Operable* detVal = nullptr; //// determine value
        Operable* detIdx = nullptr;
    };

    class Table: public Module{
    public:

        RowMeta _meta;
        const int     _identWidth = -1;
        const int     _amtSize    = -1;
        std::vector<Slot*> _hwSlots; //// for collect hardware


    explicit Table(RowMeta  meta, int identWidth = 0):
        _meta      (std::move(meta)),
        _identWidth(identWidth),
        _amtSize   (1 << identWidth){

        mfAssert(_identWidth >= 0, "table ident ");
        for (int i = 0; i < _amtSize; i++){
            Slot* slotPtr = &mOprMod(getVarName() + "_" + std::to_string(i),Slot,_meta, i);
            _hwSlots.push_back(slotPtr);
        }
    }

    ///// reqIdx used that Output require the index of table or not

    Candidate buildMinMaxLogic(int fieldIdx, bool isMin, bool reqIdx){

        ////// we will build binary tree
        FieldMeta field = _meta.getField(fieldIdx);
        int gatherSize = _amtSize; /// amount size is power of 2
        std::queue<Candidate> candidates;
        //// gather all input
        int idx = 0;
        for (Slot* slot: _hwSlots){
            Candidate cdd;
            cdd.detVal = &slot->get(fieldIdx);
            if (reqIdx){
                cdd.detIdx = &makeOprVal("detIdx", _identWidth, idx);
                idx++;
            }
            candidates.push(cdd);
        }
        int level = 0;
        while (gatherSize != 1){
            for (int i = 0; i < gatherSize; i+=2){
                /////// pop two operable
                auto a = candidates.front(); candidates.pop();
                auto b = candidates.front(); candidates.pop();
                expression& selectA = isMin ? ((*a.detVal) < (*b.detVal)): ((*a.detVal) > (*b.detVal));
                makeWire(cmp, field._fieldSize);
                cmp.addUpdateMeta(new UpdateEvent{&  selectA ,nullptr, a.detVal, DEFAULT_UE_PRI_USER});
                cmp.addUpdateMeta(new UpdateEvent{&(~selectA),nullptr, b.detVal, DEFAULT_UE_PRI_USER});
                Wire* ident = nullptr;
                if (reqIdx){
                    ident = &makeOprWire("identLevel" + std::to_string(level) + "_" + std::to_string(i), _identWidth);
                    ident->addUpdateMeta(new UpdateEvent{&  selectA ,nullptr, a.detIdx, DEFAULT_UE_PRI_USER});
                    ident->addUpdateMeta(new UpdateEvent{&(~selectA),nullptr, b.detIdx, DEFAULT_UE_PRI_USER});
                }
                candidates.push({&cmp, ident});
            }
            level++;
            gatherSize/=2;
        }
        assert(candidates.size() == 1);
        return candidates.front();
    }

    Candidate buildMinMaxLogic(const std::string& fieldName, bool isMin, bool reqIdx){
        int idx =  _meta.getFieldIdx(fieldName);
        return buildMinMaxLogic(idx, isMin, reqIdx);
    }

    SlotOpr buildGetLogic(Operable& idx){
        mfAssert(idx.getOperableSlice().getSize() == _identWidth,
            "getSlotIndexer mismatch");


        std::vector<Wire*> slotReps;

        ///// build representation wire
        for (const FieldMeta& fieldMeta: _meta.getAllFields()){
            Wire* wire = &makeOprWire("getSlot_" + fieldMeta._fieldName, fieldMeta._fieldSize);
            slotReps.push_back(wire);
        }
        ///// build representation wire
        for (int slotIdx = 0; slotIdx < _amtSize; slotIdx++){
            expression& eq = idx == slotIdx;
            /////// build assign for each field
            for (int fieldIdx = 0; fieldIdx < _meta.getSize(); fieldIdx++){
                slotReps[fieldIdx]->addUpdateMeta(new UpdateEvent{
                    &eq ,
                    nullptr,
                    &_hwSlots[slotIdx]->get(fieldIdx),
                    DEFAULT_UE_PRI_USER
                });
            }
        }

        std::vector<Operable*> cvtSlotReps;
        cvtSlotReps.reserve(slotReps.size());
        for (Wire* slotRep: slotReps){cvtSlotReps.push_back(slotRep);}
        return SlotOpr(_meta, cvtSlotReps);

    }



    };

}




#endif //__src_lib_hw_slot_TABLE_H
