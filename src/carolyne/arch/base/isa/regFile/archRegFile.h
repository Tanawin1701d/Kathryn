//
// Created by tanawin on 28/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H

#include<unordered_map>
#include<string>

#include "carolyne/arch/base/util/regFileBase.h"
#include "carolyne/arch/base/util/regType.h"


    namespace kathryn::carolyne{

        constexpr char ARF_FD_LAST_PRF_INDEX  [] = "lastInfer"; ///// index in prf that is latest rename
        constexpr char ARF_FD_COMMIT_PRF_INDEX[] = "commitedInfer"; //// index in prf that is commited


        struct ArchRegFileBase: RegFileBase, VizCsvGenable{


            ///// false is phyRegFile
            static std::string getInferFieldName(bool isCommitInfer){
                    return isCommitInfer ? (std::string(ARF_FD_LAST_PRF_INDEX) + "_" + RTM_FD_idx)
                                         : (std::string(ARF_FD_COMMIT_PRF_INDEX ) + "_" + RTM_FD_idx);

            }

            virtual RowMeta genRowMeta(const std::string& regGrpName,
                                       const RegTypeMeta& linkedPhyFile){

                /////// |lastInfer_idx|commitedInfer_idx|

                crlAss(isThereGroup(regGrpName),
                    "can't find archRegfile group name: " + regGrpName + "<=" );
                RowMeta row;
                row.addField(getInferFieldName(false), linkedPhyFile.getIndexWidth());
                row.addField(getInferFieldName(true) , linkedPhyFile.getIndexWidth());
                return row;
            }

            void visual(CsvGenFile& genFile) override{

                for (auto&[regGrpName, regTypeMeta]: _regMetas){
                    CsvTable archFileTable = regTypeMeta.genTable();
                    archFileTable.setTableName(regGrpName);
                    genFile.addData(archFileTable);
                }

            }

        };

    }





#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H
