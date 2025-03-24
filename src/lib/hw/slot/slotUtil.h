//
// Created by tanawin on 24/3/2025.
//

#ifndef KATHRYN_LIB_HW_SLOT_SLOTUTIL_H
#define KATHRYN_LIB_HW_SLOT_SLOTUTIL_H

#include "slot.h"

namespace kathryn{


    /** it build the multiplexer to select one of two slot by using selectLhs signal*/
    Slot buildSlotSelectLogic(Operable& selectLhs,
                              Slot lhsSlot,
                              Slot rhsSlot,
                              const std::string& prefixName);

}

#endif //KATHRYN_LIB_HW_SLOT_SLOTUTIL_H
