//
// Created by tanawin on 30/12/2024.
//

#ifndef src_carolyne_arch_base_march_pRegFile_PHYSICALREGFILE_H
#define src_carolyne_arch_base_march_pRegFile_PHYSICALREGFILE_H

#include "carolyne/arch/base/util/regFileBase.h"
#include "carolyne/arch/base/util/rowMeta.h"
#include "carolyne/arch/base/util/tableMeta.h"

namespace kathryn{
    namespace carolyne{

        constexpr char PRF_FD_VALUE[] = "value";
        constexpr char PRF_FD_ISVALID[] = "isValid";
        constexpr char PRF_FD_ISFREE[] = "isFree";

        struct PhysicalRegFileBase: RegFileBase{

            RowMeta buildPhyRegRowMeta(RegTypeMeta& regTypeMeta) override{
                RowMeta meta;
                ////// |isFree|isvalidBit|value|
                meta.addField(PRF_FD_VALUE  , regTypeMeta.getRegWidth());
                meta.addField(PRF_FD_ISVALID, 1);
                meta.addField(PRF_FD_ISFREE , 1);

                return meta;
            }

        };

    }

}


#endif //src_carolyne_arch_base_march_pRegFile_PHYSICALREGFILE_H
