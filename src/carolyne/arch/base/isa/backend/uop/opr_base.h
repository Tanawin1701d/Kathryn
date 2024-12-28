//
// Created by tanawin on 28/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_OPR_BASE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_OPR_BASE_H

#include<vector>
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


        /**
         * OprMatcherBase
         *  aim to match the raw instuction in each _matchedSlice
         * **/

        struct OprTypeBase;
        struct OprMatcherBase{
            int                _instrWidth = -1;
            std::vector<Slice> _matchedSlices; //// the slice in raw fetch instruction
            OprTypeBase*       _oprType = nullptr; /// the oprType that we want to match with

            OprMatcherBase(int instrWidth,
                           const std::vector<Slice>& matchedSlice,
                           OprTypeBase* oprType
                           ):
            _instrWidth(instrWidth),
            _matchedSlices(matchedSlice),
            _oprType(oprType){
                crlAss(_instrWidth > 0, "instruction width must > 0");
                for (Slice sl: _matchedSlices){
                    crlAss(sl.checkValidSlice(),
                           "invalid opr match slice " + std::to_string(sl.start) +
                           " " + std::to_string(sl.stop));
                    crlAss(sl.stop < _instrWidth,
                           "sl.stop = " + std::to_string(sl.stop) +
                           "exceed instrwidth " + std::to_string(_instrWidth));
                }
                crlAss(_oprType != nullptr, "match oprType must not be null");
            }

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


    }

}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_OPR_BASE_H
