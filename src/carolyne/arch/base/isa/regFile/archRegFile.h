//
// Created by tanawin on 28/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H

#include<unordered_map>
#include<string>

#include "carolyne/arch/base/util/regFileBase.h"
#include "carolyne/arch/base/util/regType.h"

namespace kathryn{
    namespace carolyne{

        constexpr char ARF_FD_LAST_PRF_INDEX  [] = "lastInfer"; ///// index in prf that is latest rename
        constexpr char ARF_FD_COMMIT_PRF_INDEX[] = "commitedInfer"; //// index in prf that is commited


        struct ArchRegFileBase: RegFileBase{

            int _phyIndexSize = -1;

            void setPhyIndexSize(int phyIndexSize){
                crlAss(phyIndexSize > 0, "physical index size cannot <= 0");
                _phyIndexSize = phyIndexSize;
            }

            RowMeta buildPhyRegRowMeta(RegTypeMeta& regTypeMeta) override{
                RowMeta meta;
                ////// |commitedInfer|lastInfer|
                meta.addField(ARF_FD_LAST_PRF_INDEX  , _phyIndexSize);
                meta.addField(ARF_FD_COMMIT_PRF_INDEX, _phyIndexSize);
                return meta;
            }



        };

    }



}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H
