//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_isa_uop_OPR_LI_H
#define src_carolyne_arch_base_isa_uop_OPR_LI_H
#include "carolyne/arch/base/util/regFileType.h"
#include "opr_base.h"



    namespace kathryn::carolyne{

        struct OprTypeLoadImm: OprTypeBase{


            explicit OprTypeLoadImm(ALLOC_INFO srcAllocInfo, int srcWidthBit):
                OprTypeBase(srcAllocInfo){
                _oprWidth  = srcWidthBit;
                _oprType   = COT_IMM;
                crlAss(srcWidthBit > 0, "imm width bit must greater than 0");
            }

            bool isEqualTypeDeep(const OprTypeBase& rhs) override{

                crlAss(rhs._oprType == COT_LOAD_REG_FILE,
                       "check opr deep load from regfile error because type mismatch");
                /////// we can assume that it is safely cast to this Type
                return true;
            }

        };

    }



#endif //src_carolyne_arch_base_isa_uop_OPR_LI_H
