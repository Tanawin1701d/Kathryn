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


            RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) override{
                RowMeta rowMeta;

                std::string  desArchGrpName    =  _allocInfo.relatedArchRegFile->getUtmName();
                std::string  desPhyGrpName     =  _allocInfo.relatedArchRegFile->getLinkedPhyRegFileUTM()->getUtmName();
                RegTypeMeta& desArcRegtypeMeta = *_allocInfo.relatedArchRegFile;
                RegTypeMeta& desPhyRegtypeMeta = *_allocInfo.relatedArchRegFile->getLinkedPhyRegFileUTM();

                switch (genMode){

                    case CGM_DECODE :{
                        if (isThere(_allocInfo.regAllocOption, REG_OPT::REG_REQ_ARCH_RENAME)){
                            rowMeta.addField(
                                genOprFieldName(OPR_FD_LOAD_REG_FD_archIdx, desArchGrpName),
                                desArcRegtypeMeta.getIndexWidth());
                        }
                        break;
                    }
                    case CGM_ALLOC :{

                        if (isThere(_allocInfo.regAllocOption, REG_OPT::REG_REQ_ARCH_RENAME)){
                            rowMeta.addField(
                                genOprFieldName(OPR_FD_LOAD_REG_FD_archIdx, desArchGrpName),
                                desArcRegtypeMeta.getIndexWidth());
                        }
                        if (isThere(_allocInfo.regAllocOption, REG_OPT::REG_REQ_PHY_ALLOC)){
                            rowMeta.addField(
                                genOprFieldName(OPR_FD_LOAD_REG_FD_phyIdx, desPhyGrpName),
                                desPhyRegtypeMeta.getIndexWidth());
                        }
                        break;
                    }
                    case CGM_RSV :{
                        if(isThere(_allocInfo.regAllocOption, REG_OPT::REG_REQ_PHY_ALLOC)){
                            rowMeta.addField(
                                genOprFieldName(OPR_FD_LOAD_REG_FD_phyIdx, desPhyGrpName),
                                desPhyRegtypeMeta.getIndexWidth());
                        }
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
