//
// Created by tanawin on 16/1/2025.
//

#ifndef LIB_HW_SLOT_SLOT_H
#define LIB_HW_SLOT_SLOT_H

#include <utility>

#include "model/controller/controller.h"
#include "rowMeta.h"

namespace kathryn{


    struct SlotMeta{
        Operable*   opr = nullptr;
        Assignable* asb = nullptr;

    };

    class Slot{
    public:
        /**metadata zone*/
        bool    _readOnly;
        RowMeta _meta ; ////
        int     _ident; ////
        /**hardware zone*/
        std::vector<SlotMeta> _hwMetas;

        explicit Slot(RowMeta meta, int identVal):
        _readOnly(false),
        _meta    (std::move(meta)),
        _ident   (identVal){}

        explicit Slot(RowMeta meta, const std::vector<SlotMeta>& slotMetas, int identVal = -1):
        _readOnly(false),
        _meta    (std::move(meta)),
        _ident   (identVal),
        _hwMetas (slotMetas){
            mfAssert(!slotMetas.empty(), "cannot create slot with empt slot Metas");
            if (slotMetas[0].asb == nullptr){_readOnly = true;}
            integrityCheck();
        }

        explicit Slot(const std::vector<std::string>& fieldName,
                      const std::vector<Operable*>& oprs, int identVal = -1):
        _readOnly(true),
        _ident(identVal){

            ///// collect size for each filed
            std::vector<int> sizes;
            for (Operable* opr: oprs){
                mfAssert(opr != nullptr, "cannot construct slot from nullptr");
                int sz = opr->getOperableSlice().getSize();
                sizes.push_back(sz);
            }
            _meta = RowMeta(fieldName, sizes);
            ///// build _hwMetas
            for (Operable* opr: oprs){
                _hwMetas.push_back({opr, nullptr});
            }

        }

        /** checking idx*/
        [[nodiscard]]
        bool isContainIdx(int startIdx, int stopIdx)const{
            mfAssert(startIdx < stopIdx, "isContainIdx have invalid input");
            return _meta.isThereIdx(startIdx) && _meta.isThereIdx(stopIdx-1);
        }
        bool integrityCheck(){
            if (_hwMetas.size() != _meta.getAmtField()){return false;}
            for (int idx = 0; idx < _meta.getAmtField(); idx++){
                if (_meta.getField(idx)._fieldSize != _hwMetas[idx].opr->getOperableSlice().getSize()){
                    return false;
                }
            }
            return true;
        }

        Slot operator() (int startIdx, int stopIdx)const{

            mfAssert(isContainIdx(startIdx, stopIdx),
                "slice slot opr index out of range");

            //////// build new Slot ----->> sliced slot
            std::vector<SlotMeta> slotMetas;
            for (int i = startIdx; i < stopIdx; i++){
                slotMetas.push_back(_hwMetas[i]);
            }
            Slot slicedSlot(_meta(startIdx, stopIdx), slotMetas, _ident);
            return slicedSlot;

        }

        Slot operator() (const std::string& startName, const std::string& stopName) const{
            int startIdx = _meta.getFieldIdx(startName);
            int stopIdx  = _meta.getFieldIdx(stopName) + 1;
            return operator()(startIdx, stopIdx);
        }

        Slot operator() (const std::vector<std::string>& fieldNames){
            ///// 1. build new row meta  (field meta-data)
            ///// 2. build new slot meta (hardware)
            std::vector<FieldMeta> fms;
            std::vector<SlotMeta>  slotMetas;
            for (auto& fieldName: fieldNames){
                int idxInCurSlot = _meta.getFieldIdx(fieldName);
                fms      .push_back(_meta.getField(idxInCurSlot));
                slotMetas.push_back(_hwMetas[idxInCurSlot]);
            }
            return Slot(RowMeta(fms), slotMetas, _ident);
        }

        Slot operator() (const RowMeta& rowMeta){
            return operator() (rowMeta.getAllFieldNames());
        }

        Operable& at(const std::string& fieldName); /// TODO make this


        Slot operator+ (const Slot& rhs) const{
            ////// todo pool slot
            RowMeta               newRM      = _meta; newRM += rhs._meta;
            int                   newIdent   = _ident;
            std::vector<SlotMeta> newHwMetas = _hwMetas;
            for (const SlotMeta& rSlotMeta: rhs._hwMetas){
                newHwMetas.push_back(rSlotMeta);
            }
            return Slot(newRM, newHwMetas, newIdent);
        }

        void setNewRowMeta(const RowMeta& newRowMeta){
            _meta = newRowMeta;
            integrityCheck();
        }

        [[nodiscard]]
        int getAmtField()const{return _meta.getAmtField();}

        [[nodiscard]]
        RowMeta getMeta()const{return _meta;}

        /**  assigning system*/

        /////////// main assigning
        void assignCore(const Slot& rhsSlot, bool isBlockAsm){

            int srcIdx = 0;
            ////////// loop to all field and assign the value which the name is matched
            for (const FieldMeta& fm: rhsSlot.getMeta().getAllFields()){
                int desIdx = _meta.getFieldIdx(fm._fieldName);
                mfAssert(isContainIdx(desIdx, desIdx+1), "cannot assign fieldName: " + fm._fieldName);
                Slice desSlice = _hwMetas[desIdx].opr->getOperableSlice();
                if (isBlockAsm) {
                    _hwMetas[desIdx].asb->doBlockAsm(*rhsSlot._hwMetas[srcIdx].opr, desSlice);
                }else{
                    _hwMetas[desIdx].asb->doNonBlockAsm(*rhsSlot._hwMetas[srcIdx].opr, desSlice);
                }
                srcIdx += 1;
            }
        }

        void assignCore(const std::string& startFieldName,
                        Operable& dayta,
                        bool      isBlockAsm){
            RowMeta rm({startFieldName}, {dayta.getOperableSlice().getSize()});
            Slot rhsSlot(rm, {{&dayta, nullptr}}, -1);
            assignCore(rhsSlot, isBlockAsm);
        }

        Slot& operator <<= (const std::vector<uint64_t>& rhsInts); //// TODO make it be int assign for easi
        Slot& operator =   (const std::vector<uint64_t>& rhsInts);
        Slot& operator <<= (uint64_t rhsVal);
        Slot& operator =   (uint64_t rhsVal);
        Slot& operator <<= (Operable& rhsOpr);
        Slot& operator =   (Operable& rhsOpr);
        Slot&          bls (const std::vector<Operable*>& rhsOprs);
        Slot&          nbls(const std::vector<Operable*>& rhsOprs);
        Slot& operator <<= (const std::vector<Operable*>& rhsOprs);
        Slot& operator =   (const std::vector<Operable*>& rhsOprs);
        Slot& operator <<= (const Slot& rhsSlot){ assignCore(rhsSlot, true ); return *this;}
        Slot& operator =   (const Slot& rhsSlot){ assignCore(rhsSlot, false); return *this;}


        /** hardware getter system*/
        SlotMeta getHwSlotMeta(const std::string& fieldName){
            int idx = _meta.getFieldIdx(fieldName);
            return getHwSlotMeta(idx);
        }
        SlotMeta getHwSlotMeta(int idx){
            mfAssert(isContainIdx(idx, idx+1), "cannot find fieldIdx " + std::to_string(idx));
            return _hwMetas[idx];
        }

        Operable& getField(const std::string& fieldName){
            return *getHwSlotMeta(fieldName).opr;
        }
        [[nodiscard]]
        int       getIdent() const {return _ident;}

    };

    class RegSlot: public Slot{
    public:
        std::vector<Reg*> hwFields;

        ///////// constructor
        explicit RegSlot(const RowMeta& meta, int identVal = -1, const std::string& suffix = ""):
        Slot(meta,identVal){

            for (const auto& field: meta.getAllFields()){
                hwFields.push_back(&mOprReg(field._fieldName + "_ident_" + std::to_string(_ident) + suffix,
                                            field._fieldSize));
                _hwMetas .push_back({*hwFields.rbegin(), *hwFields.rbegin()});
            }
        }

        explicit RegSlot(const std::vector<std::string> &nms,
                         const std::vector<int> &szs):
        RegSlot(RowMeta(nms, szs)){}

        /** hardware getter system*/
        Reg& get(const std::string& fieldName){
            int idx = _meta.getFieldIdx(fieldName);
            return get(idx);
        }
        Reg& get(int idx){
            mfAssert(idx >= 0 && idx < hwFields.size(), "cannot find fieldIdx " + std::to_string(idx));
            return *hwFields[idx];
        }
    };

    class WireSlot: public Slot{
    public:
        std::vector<Wire*> hwFields;

        explicit WireSlot(const RowMeta& meta, int identVal = -1):
        Slot(meta,identVal){
            for (const auto& field: meta.getAllFields()){
                hwFields.push_back(&mOprWire(field._fieldName + "_ident_" + std::to_string(_ident), field._fieldSize));
                _hwMetas .push_back({*hwFields.rbegin(), *hwFields.rbegin()});
            }
        }

        /** hardware getter system*/

        Wire& get(const std::string& fieldName){
            int idx = _meta.getFieldIdx(fieldName);
            return get(idx);
        }

        Wire& get(int idx){
            mfAssert(idx >= 0 && idx < hwFields.size(), "cannot find fieldIdx " + std::to_string(idx));
            return *hwFields[idx];
        }
    };

}

#endif //LIB_HW_SLOT_SLOT_H
