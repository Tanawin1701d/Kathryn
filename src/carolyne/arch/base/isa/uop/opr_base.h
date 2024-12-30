//
// Created by tanawin on 28/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_OPR_BASE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_OPR_BASE_H

#include<vector>

#include "carolyne/arch/base/util/sliceMatcher.h"
#include "model/hwComponent/abstract/Slice.h"
#include "carolyne/util/checker/checker.h"

namespace kathryn{

    namespace carolyne{

        enum CRL_OPR_TYPE{
            COT_REG_FILE = 0,
            COT_IMM      = 1,
            COT_OTHER    = 2,
            COT_CNT      = 3
        };

        struct OprTypeBase{
            int oprWidth = -1;
            CRL_OPR_TYPE oprType = COT_CNT;
            int subType = -1; /// it will be used when oprType == COT_OTHER

            virtual ~OprTypeBase(){}

            virtual bool isEqualTypeDeep(const OprTypeBase& rhs)  = 0;

            bool isEqualType(const OprTypeBase& rhs){
                bool prelimCheck = (oprWidth == rhs.oprWidth) &&
                                   (oprType  == rhs.oprType)  &&
                                   (subType  == rhs.subType);
                if (!prelimCheck) {
                    return false;
                }
                return isEqualTypeDeep(rhs);
            }
        };



        /**
         * OprMatcherBase
         *  aim to match the raw instuction in each _matchedSlice
         * **/

        struct OprMatcherBase: SliceMatcher{
            int                _instrWidth = -1;
            OprTypeBase*       _oprType = nullptr; /// the oprType that we want to match with

            OprMatcherBase(int instrWidth, OprTypeBase* oprType):
            SliceMatcher(instrWidth),
            _instrWidth(instrWidth),
            _oprType(oprType){
                crlAss(_instrWidth > 0, "instruction width must > 0");
                crlAss(_oprType != nullptr, "match oprType must not be null");
            }

        };

    }
}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_OPR_BASE_H
