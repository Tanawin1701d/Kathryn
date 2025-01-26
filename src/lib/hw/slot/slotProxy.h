//
// Created by tanawin on 17/1/2025.
//

#ifndef src_lib_hw_slot_SLOTPROXY_H
#define src_lib_hw_slot_SLOTPROXY_H
#include <utility>

#include "rowMeta.h"
#include "model/controller/controller.h"



namespace kathryn{


    ////// it is used to represent Slot but not make any new register
    class SlotOpr{
    public:
        /**metadata zone*/
        RowMeta _meta;
        /**hardware zone*/
        std::vector<Operable*> _repFields;

        explicit SlotOpr(RowMeta  meta,
                         const std::vector<Operable*>& repFields):
        _meta     (std::move(meta)),
        _repFields(repFields){}


        /** hardware getter **/

        Operable*& getFieldRef(int idx){
            mfAssert(idx >= 0 && idx < _repFields.size(),
                "getFieldRef with idx is out of range " + std::to_string(idx));
            return _repFields[idx];
        }

        Operable*& getFieldRef(const std::string& fieldName){
            int idx = _meta.getFieldIdx(fieldName);
            return getFieldRef(idx);
        }

        Operable& getOpr(int idx){
            mfAssert(_meta.isThereIdx(idx), "cannot get opr from index " + std::to_string(idx));
            return *_repFields[idx];
        }

        Operable& getOpr(const std::string& fieldName){
            int idx = _meta.getFieldIdx(fieldName);
            return getOpr(idx);
        }

        /** meta-data getter**/

        RowMeta getRowMeta() const{
            return _meta;
        }

        FieldMeta getFieldMeta(int idx) const{
            return _meta.getField(idx);
        }

        [[nodiscard]]
        RowMeta getMeta() const{
            return _meta;
        }

        [[nodiscard]]
        int getAmtField()const{
            return static_cast<int>(_repFields.size());
        }

    };

}



#endif //src_lib_hw_slot_SLOTPROXY_H
