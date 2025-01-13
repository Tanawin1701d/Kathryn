//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_isa_uop_OPR_SRF_H
#define src_carolyne_arch_base_isa_uop_OPR_SRF_H

#include "opr_base.h"
#include "carolyne/arch/base/util/regType.h"




    namespace kathryn::carolyne{

        struct OprTypeStoreRegFile: OprTypeBase{

            explicit OprTypeStoreRegFile(
            const APRegRobFieldMatch&    desAPRegTypeMatch,
                  ArchRegFileBase*       archRegFiles,
                  PhysicalRegFileBase*   phyRegFiles):
            OprTypeBase(APRegRobFieldMatch(),
                        desAPRegTypeMatch,
                        archRegFiles,
                        phyRegFiles)
            {
                ///// for now we assume archRegType and phyRegType is correct
                RegTypeMeta desArcRegtypeMeta = _archRegFiles->getRegTypeMetaGroup(_desAPRegTypeMatch._archRegGrpName);
                _oprWidth = desArcRegtypeMeta.REG_WIDTH;
                _oprType  = COT_LOAD_REG_FILE;
            }

            bool isEqualTypeDeep(const OprTypeBase& rhs) override{

                crlAss(rhs._oprType == COT_LOAD_REG_FILE,
                    "check opr deep load from regfile error because type mismatch");
                /////// we can assume that it is safely cast to this Type
                return true;

            }


            RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) override{
                RowMeta rowMeta;

                std::string desArchGrpName    = _desAPRegTypeMatch._archRegGrpName;
                std::string desPhyGrpName     = _desAPRegTypeMatch._phyRegGrpName;
                RegTypeMeta desArcRegtypeMeta = _archRegFiles->getRegTypeMetaGroup(desArchGrpName);
                RegTypeMeta desPhyRegtypeMeta = _phyRegFiles->getRegTypeMetaGroup (desPhyGrpName);

                switch (genMode){

                    case CGM_DECODE :{
                        //// valid is model responsibility
                        ///rowMeta.addField(OPR_FD_LOAD_REG_FD_valid  , 1);
                        rowMeta.addField(
                            genTypeWithGrpName(OPR_FD_LOAD_REG_FD_archIdx, desArchGrpName),
                            desArcRegtypeMeta.getIndexWidth());
                        break;
                    }
                    case CGM_ALLOC :{
                        ///rowMeta.addField(OPR_FD_LOAD_REG_FD_valid  , 1);
                        rowMeta.addField(
                            genTypeWithGrpName(OPR_FD_LOAD_REG_FD_archIdx, desArchGrpName),
                            desArcRegtypeMeta.getIndexWidth());
                        rowMeta.addField(
                            genTypeWithGrpName(OPR_FD_LOAD_REG_FD_phyIdx, desPhyGrpName),
                            desPhyRegtypeMeta.getIndexWidth());
                        break;
                    }
                    case CGM_RSV :{
                            ///rowMeta.addField(OPR_FD_LOAD_REG_FD_valid  , 1);
                            rowMeta.addField(
                                genTypeWithGrpName(OPR_FD_LOAD_REG_FD_phyIdx, desPhyGrpName),
                                desPhyRegtypeMeta.getIndexWidth());
                            break;
                    }

                    default:{
                        crlAss(false, "OprTypeLoadRegFile out of row meta generation");
                    }
                }
                return rowMeta;
            }

        };
    }


#endif //src_carolyne_arch_base_isa_uop_OPR_SRF_H
