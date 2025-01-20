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

            explicit OprTypeLoadRegFile(const APRegRobFieldMatch&  srcAPRegTypeMatch):
            OprTypeBase(srcAPRegTypeMatch,
                        APRegRobFieldMatch()){
                ///// for now we assume archRegType and phyRegType is correct
                _oprWidth = srcAPRegTypeMatch.relatedArchRegFile->REG_WIDTH;
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
                std::string  srcArchGrpName    =  _srcAPRegTypeMatch.relatedArchRegFile->getUtmName();
                std::string  srcPhyGrpName     =  _srcAPRegTypeMatch.relatedArchRegFile->getLinkedPhyRegFileUTM()->getUtmName();
                RegTypeMeta& srcArcRegtypeMeta = *_srcAPRegTypeMatch.relatedArchRegFile;
                RegTypeMeta& srcPhyRegtypeMeta = *_srcAPRegTypeMatch.relatedArchRegFile->getLinkedPhyRegFileUTM();

                switch (genMode){

                    case CGM_DECODE :{
                        //// valid is model responsibility
                        ///rowMeta.addField(OPR_FD_LOAD_REG_FD_valid  , 1);
                        rowMeta.addField(
                        genOprFieldName(OPR_FD_LOAD_REG_FD_archIdx, srcArchGrpName),
                        srcArcRegtypeMeta.getIndexWidth());
                        break;
                    }
                    case CGM_ALLOC :{
                        ///rowMeta.addField(OPR_FD_LOAD_REG_FD_valid  , 1);
                        rowMeta.addField(
                        genOprFieldName(OPR_FD_LOAD_REG_FD_archIdx, srcArchGrpName),
                        srcArcRegtypeMeta.getIndexWidth());
                        rowMeta.addField(
                        genOprFieldName(OPR_FD_LOAD_REG_FD_phyIdx, srcPhyGrpName),
                        srcPhyRegtypeMeta.getIndexWidth());
                        break;
                    }
                    case CGM_RSV :{
                            ///rowMeta.addField(OPR_FD_LOAD_REG_FD_valid  , 1);
                            rowMeta.addField(
                        genOprFieldName(OPR_FD_LOAD_REG_FD_phyIdx, srcPhyGrpName),
                        srcPhyRegtypeMeta.getIndexWidth());
                            rowMeta.addField(
                        genOprFieldName(OPR_FD_LOAD_REG_FD_value, srcPhyGrpName),
                        srcPhyRegtypeMeta.getRegWidth());
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


#endif //src_carolyne_arch_base_isa_uop_OPR_SAMPLE_H
