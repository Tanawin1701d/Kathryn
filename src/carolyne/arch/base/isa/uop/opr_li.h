//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_isa_uop_OPR_LI_H
#define src_carolyne_arch_base_isa_uop_OPR_LI_H
#include "carolyne/arch/base/util/regFileType.h"
#include "opr_base.h"



    namespace kathryn::carolyne{

        struct OprTypeLoadImm: OprTypeBase{


            explicit OprTypeLoadImm(
                const APRegRobFieldMatch&  desAPRegTypeMatch,
                ArchRegFileBase*       archRegFiles,
                PhysicalRegFileBase*   phyRegFiles):
                OprTypeBase(APRegRobFieldMatch(),
                            desAPRegTypeMatch,
                            archRegFiles,
                            phyRegFiles){

                RegTypeMeta desArcRegtypeMeta = _archRegFiles->getRegTypeMetaGroup(_desAPRegTypeMatch._archRegGrpName);
                _oprWidth  = desArcRegtypeMeta.REG_WIDTH;
                _oprType   = COT_IMM;
            }

            bool isEqualTypeDeep(const OprTypeBase& rhs) override{

                crlAss(rhs._oprType == COT_LOAD_REG_FILE,
                       "check opr deep load from regfile error because type mismatch");
                /////// we can assume that it is safely cast to this Type
                return true;
            }

            RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) override{
                RowMeta rowMeta;
                switch (genMode){

                    case CGM_DECODE :
                    case CGM_ALLOC :
                    case CGM_RSV :{
                        //// valid is model responsibility
                        ///rowMeta.addField(OPR_FD_LOAD_REG_FD_valid  , 1);
                        RegTypeMeta desArcRegtypeMeta = _archRegFiles->getRegTypeMetaGroup(_desAPRegTypeMatch._archRegGrpName);
                        rowMeta.addField(
                            genTypeWithGrpName(OPR_FD_LOAD_REG_FD_value, "imm"),
                            desArcRegtypeMeta.getRegWidth());
                        break;
                    }
                    default:{crlAss(false, "OprTypeLoadRegFile out of row meta generation");}
                }
                return rowMeta;
            }

        };

    }



#endif //src_carolyne_arch_base_isa_uop_OPR_LI_H
