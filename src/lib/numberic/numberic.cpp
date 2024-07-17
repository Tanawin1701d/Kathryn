//
// Created by tanawin on 16/7/2024.
//

#include "numberic.h"

namespace kathryn{

    Operable& ext(Operable& value, int targetSize, Operable& sign){
        assert(targetSize > value.getOperableSlice().getSize());
        assert(value.getOperableSlice().getSize() > 0);
        assert(sign.getOperableSlice().getSize() == 1);

        int amtToExtend = targetSize - value.getOperableSlice().getSize();
                                /////   v---- lsb                  v----- msb
        return gManInternalReadOnly( {&value, &((sign & 1).extB(amtToExtend))});

    }

}