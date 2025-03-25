//
// Created by tanawin on 24/3/2025.
//

#include "slotUtil.h"

namespace kathryn{

    Slot buildSlotSelectLogic(Operable& selectLhs,
                              Slot lhsSlot,
                              Slot rhsSlot,
                              const std::string& prefixName){ //// for debug purpose only

        ////// the selected wire will be placed at selectedSlotMeta
        std::vector<SlotMeta> selectedSlotMeta;

        for(const FieldMeta& fieldMeta: lhsSlot.getMeta().getAllFields()){
            ///// build new selected wire for each wire in each field
            Wire& joinWire = mOprWire(prefixName + "_"+ "selected" + fieldMeta._fieldName, fieldMeta._fieldSize);
            joinWire.addUpdateMeta(
                    new UpdateEvent{
                            &selectLhs ,
                            nullptr,
                            lhsSlot.getHwSlotMeta(fieldMeta._fieldName).opr,
                            lhsSlot.getHwSlotMeta(fieldMeta._fieldName).opr->getOperableSlice(),
                            DEFAULT_UE_PRI_USER});
            joinWire.addUpdateMeta(
                    new UpdateEvent{
                            &(~selectLhs),
                            nullptr,
                            rhsSlot.getHwSlotMeta(fieldMeta._fieldName).opr,
                            rhsSlot.getHwSlotMeta(fieldMeta._fieldName).opr->getOperableSlice(),
                            DEFAULT_UE_PRI_USER});
            selectedSlotMeta.push_back({&joinWire, &joinWire});
        }
        //////// build new slot
        return Slot(lhsSlot.getMeta(), selectedSlotMeta, -1);
    }

}