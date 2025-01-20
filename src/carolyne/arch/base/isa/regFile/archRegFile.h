//
// Created by tanawin on 28/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H

#include<unordered_map>
#include<string>

#include "carolyne/arch/base/util/regFileType.h"
#include "carolyne/arch/base/util/genRowMeta.h"
#include "carolyne/arch/base/util/metaIdent.h"


namespace kathryn::carolyne{

        constexpr char ARF_FD_LAST_PRF_INDEX  [] = "lastInfer"; ///// index in prf that is latest rename
        constexpr char ARF_FD_COMMIT_PRF_INDEX[] = "commitedInfer"; //// index in prf that is commited


        struct PhyRegFileUTM;
        struct ArchRegFileUTM: RegTypeMeta  , GenRowMetaable,
                               VizCsvGenable, MetaIdentifiable{

            PhyRegFileUTM* _linkedPhyRegFile = nullptr;

            explicit ArchRegFileUTM(int indexSize, int regWidth,
                                    const std::string &typeName) :
            RegTypeMeta(indexSize,regWidth),
            MetaIdentifiable(typeName){}

            ///// false is phyRegFile
            static std::string getInferFieldName(bool isCommitInfer){
                    return isCommitInfer ? (std::string(ARF_FD_LAST_PRF_INDEX  ) + "_" + RTM_FD_idx)
                                         : (std::string(ARF_FD_COMMIT_PRF_INDEX) + "_" + RTM_FD_idx);
            }

            void linkPhyRegFileUTM(PhyRegFileUTM* phyRegFile){
                crlAss(phyRegFile != nullptr, "cannot link archRegFile with nullptr");
                _linkedPhyRegFile = phyRegFile;
            }

            virtual RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) override;

            void visual(CsvGenFile& genFile) override{
                genFile.addData(RegTypeMeta::genTable());
            }

        };

    }





#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H
