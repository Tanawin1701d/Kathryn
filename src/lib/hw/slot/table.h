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

    ///// reqIdx used that Output require the index of table or not

    Candidate buildMinMaxLogic(int fieldIdx, bool isMin, bool reqIdx){

        ////// we will build binary tree
        FieldMeta field = _meta.getField(fieldIdx);
        int gatherSize = _amtSize; /// amount size is power of 2
        std::queue<Candidate> candidates;
        //// gather all input candidate to queue
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
        //// build compare tree
        int level = 0;
        while (gatherSize != 1){
            for (int i = 0; i < gatherSize; i+=2){
                /////// pop two operable
                auto a = candidates.front(); candidates.pop();
                auto b = candidates.front(); candidates.pop();
                ////// select candidate value to cmp
                expression& selectA = isMin ? ((*a.detVal) <= (*b.detVal)): ((*a.detVal) >= (*b.detVal));
                makeWire(cmp, field._fieldSize);
                cmp.addUpdateMeta(new UpdateEvent{&  selectA ,nullptr, a.detVal, cmp.getOperableSlice(),DEFAULT_UE_PRI_USER});
                cmp.addUpdateMeta(new UpdateEvent{&(~selectA),nullptr, b.detVal, cmp.getOperableSlice(),DEFAULT_UE_PRI_USER});
                ////// select index coresponding to the cmpResult
                Wire* ident = nullptr;
                if (reqIdx){
                    ident = &makeOprWire(_tableName + "_identLevel" + std::to_string(level) + "_" + std::to_string(i), _identWidth);
                    ident->addUpdateMeta(new UpdateEvent{&  selectA ,nullptr, a.detIdx, ident->getOperableSlice(),DEFAULT_UE_PRI_USER});
                    ident->addUpdateMeta(new UpdateEvent{&(~selectA),nullptr, b.detIdx, ident->getOperableSlice(),DEFAULT_UE_PRI_USER});
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
