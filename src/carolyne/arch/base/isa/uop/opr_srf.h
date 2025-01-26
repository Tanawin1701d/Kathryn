//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_isa_uop_OPR_SRF_H
#define src_carolyne_arch_base_isa_uop_OPR_SRF_H

#include "opr_base.h"
#include "carolyne/arch/base/util/regFileType.h"




    namespace kathryn::carolyne{

        struct OprTypeStoreRegFile: OprTypeBase{

            explicit OprTypeStoreRegFile(const ALLOC_INFO&    desAllocInfo):
            OprTypeBase(desAllocInfo)
            {
                ///// for now we assume archRegType and phyRegType is correct
                _oprWidth = desAllocInfo.relatedArchRegFile->getIndexWidth();
                _oprType  = COT_LOAD_REG_FILE;
            }

            bool isEqualTypeDeep(const OprTypeBase& rhs) override{

                crlAss(rhs._oprType == COT_LOAD_REG_FILE,
                    "check opr deep load from regfile error because type mismatch");
                /////// we can assume that it is safely cast to this Type
                return true;

            }

        };
    }


#endif //src_carolyne_arch_base_isa_uop_OPR_SRF_H
