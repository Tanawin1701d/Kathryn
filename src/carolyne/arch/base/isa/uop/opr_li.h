//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_isa_uop_OPR_LI_H
#define src_carolyne_arch_base_isa_uop_OPR_LI_H
#include "carolyne/arch/base/util/regType.h"

#include "opr_base.h"

namespace kathryn{

    namespace carolyne{

        struct OprTypeLoadImm: OprTypeBase{

            RegTypeMeta _archRegType;

            explicit OprTypeLoadImm(const RegTypeMeta& archRegType):
                _archRegType(archRegType){
                oprWidth  = archRegType.REG_WIDTH;
                oprType   = COT_IMM;
            }

            bool isEqualTypeDeep(const OprTypeBase& rhs) override{

                crlAss(rhs.oprType == COT_LOAD_REG_FILE,
                       "check opr deep load from regfile error because type mismatch");
                /////// we can assume that it is safely cast to this Type
                auto* castedRhs = (OprTypeLoadImm*)(&rhs);

                return (_archRegType == castedRhs->_archRegType);
            }

            RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) override{
                RowMeta rowMeta;
                switch (genMode){

                    case CGM_DECODE :
                    case CGM_ALLOC :
                    case CGM_RSV :{
                        //// valid is model responsibility
                        ///rowMeta.addField(OPR_FD_LOAD_REG_FD_valid  , 1);
                        rowMeta.addField(
                            genTypeWithGrpName(OPR_FD_LOAD_REG_FD_value, "imm"),
                            _archRegType.getRegWidth());
                        break;
                    }
                    default:{crlAss(false, "OprTypeLoadRegFile out of row meta generation");}
                }
                return rowMeta;
            }

        };

    }

}

#endif //src_carolyne_arch_base_isa_uop_OPR_LI_H
