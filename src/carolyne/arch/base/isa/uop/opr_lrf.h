//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_isa_uop_OPR_SAMPLE_H
#define src_carolyne_arch_base_isa_uop_OPR_SAMPLE_H
#include "carolyne/arch/base/util/regFileType.h"

#include "opr_base.h"


    namespace kathryn::carolyne{

        //// this operand load the value from regfile
        struct OprTypeLoadRegFile: OprTypeBase{

            explicit OprTypeLoadRegFile(const ALLOC_INFO&  srcAllocInfo):
            OprTypeBase(srcAllocInfo){
                ///// for now we assume archRegType and phyRegType is correct
                _oprWidth = srcAllocInfo.relatedArchRegFile->REG_WIDTH;
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


#endif //src_carolyne_arch_base_isa_uop_OPR_SAMPLE_H
