//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_isa_uop_OPR_SAMPLE_H
#define src_carolyne_arch_base_isa_uop_OPR_SAMPLE_H
#include "carolyne/arch/base/util/regType.h"

#include "opr_base.h"

namespace kathryn{
    namespace carolyne{

        //// this operand load the value from regfile
        struct OprTypeLoadRegFile: OprTypeBase{

            RegTypeMeta _archRegType;
            std::string _archRegGroupName;
            RegTypeMeta _phyRegType;
            std::string _phyRegGroupName;


            explicit OprTypeLoadRegFile(
                const RegTypeMeta& archRegType,
                const std::string& archRegGroupName,
                const RegTypeMeta& phyRegType,
                const std::string& phyRegGroupName):
            _archRegType(archRegType),
            _archRegGroupName(archRegGroupName),
            _phyRegType(phyRegType),
            _phyRegGroupName(phyRegGroupName)
            {
                ///// for now we assume archRegType and phyRegType is correct
                oprWidth = archRegType.REG_WIDTH;
                oprType  = COT_LOAD_REG_FILE;
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
                            rowMeta.addField(
                                genTypeWithGrpName(OPR_FD_LOAD_REG_FD_value, _phyRegGroupName),
                                _phyRegType.getRegWidth());
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

#endif //src_carolyne_arch_base_isa_uop_OPR_SAMPLE_H
