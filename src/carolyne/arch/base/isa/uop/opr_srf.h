//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_isa_uop_OPR_SRF_H
#define src_carolyne_arch_base_isa_uop_OPR_SRF_H

#include "opr_base.h"
#include "carolyne/arch/base/util/regType.h"



namespace kathryn{
    namespace carolyne{
        struct OprTypeStoreRegFile: OprTypeBase{

            RegTypeMeta _archRegType;
            std::string _archRegGroupName;
            RegTypeMeta _phyRegType;
            std::string _phyRegGroupName;


            explicit OprTypeStoreRegFile(
                const RegTypeMeta& archRegType,
                const std::string& archRegGroupName,
                const RegTypeMeta& phyRegType,
                const std::string& phyRegGroupName):
            _archRegType      (archRegType),
            _archRegGroupName (archRegGroupName),
            _phyRegType       (phyRegType),
            _phyRegGroupName  (phyRegGroupName)
            {
                ///// for now we assume archRegType and phyRegType is correct
                oprWidth = archRegType.REG_WIDTH;
                oprType  = COT_LOAD_REG_FILE;
            }

            bool isEqualTypeDeep(const OprTypeBase& rhs) override{

                crlAss(rhs.oprType == COT_LOAD_REG_FILE,
                    "check opr deep load from regfile error because type mismatch");
                /////// we can assume that it is safely cast to this Type
                auto* castedRhs = (OprTypeStoreRegFile*)(&rhs);

                return
                (_archRegType        ==    castedRhs->_archRegType      ) &&
                (_archRegGroupName   ==    castedRhs->_archRegGroupName ) &&
                (_phyRegType         ==    castedRhs->_phyRegType       ) &&
                (_phyRegGroupName    ==    castedRhs->_phyRegGroupName  );

            }


            RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) override{
                RowMeta rowMeta;
                switch (genMode){

                    case CGM_DECODE :{
                        //// valid is model responsibility
                        ///rowMeta.addField(OPR_FD_LOAD_REG_FD_valid  , 1);
                        rowMeta.addField(
                            genTypeWithGrpName(OPR_FD_LOAD_REG_FD_archIdx, _archRegGroupName),
                            _archRegType.getIndexWidth());
                        break;
                    }
                    case CGM_ALLOC :{
                        ///rowMeta.addField(OPR_FD_LOAD_REG_FD_valid  , 1);
                        rowMeta.addField(
                            genTypeWithGrpName(OPR_FD_LOAD_REG_FD_archIdx, _archRegGroupName),
                            _archRegType.getIndexWidth());
                        rowMeta.addField(
                            genTypeWithGrpName(OPR_FD_LOAD_REG_FD_phyIdx, _phyRegGroupName),
                            _phyRegType.getIndexWidth());
                        break;
                    }
                    case CGM_RSV :{
                            ///rowMeta.addField(OPR_FD_LOAD_REG_FD_valid  , 1);
                            rowMeta.addField(
                                genTypeWithGrpName(OPR_FD_LOAD_REG_FD_phyIdx, _phyRegGroupName),
                                _phyRegType.getIndexWidth());
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
}

#endif //src_carolyne_arch_base_isa_uop_OPR_SRF_H
