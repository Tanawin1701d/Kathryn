//
// Created by tanawin on 28/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_OPR_BASE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_OPR_BASE_H

#include <utility>
#include<vector>
#include "lib/hw/slot/slot.h"
#include "model/hwComponent/abstract/Slice.h"
#include "carolyne/arch/base/march/alloc/allocInfo.h"
#include "carolyne/arch/base/march/prfUnit/prfMetaBase.h"
#include "carolyne/arch/base/util/genRowMeta.h"
#include "carolyne/arch/base/util/sliceMatcher.h"
#include "carolyne/util/checker/checker.h"

    namespace kathryn::carolyne{

        constexpr char OPR_FD_LOAD_REG_FD_valid   [] = "valid";
        constexpr char OPR_FD_LOAD_REG_FD_archIdx [] = "archIdx";
        constexpr char OPR_FD_LOAD_REG_FD_phyIdx  [] = "phyIdx";
        constexpr char OPR_FD_LOAD_REG_FD_value   [] = "value";

        /**opr type will associate with allocation */
        enum CRL_OPR_TYPE{
            COT_LOAD_REG_FILE  = 0,
            COT_STORE_REG_FILE = 1,
            COT_IMM            = 2,
            COT_OTHER          = 3,
            COT_CNT            = 4
        };

        struct OprTypeBase{
            /** archReg phyReg*/
            ALLOC_INFO         _allocInfo;
            /** opr width*/
            int                _oprWidth = -1;
            CRL_OPR_TYPE       _oprType = COT_CNT;
            int                _subType = -1; /// it will be used when oprType == COT_OTHER

            explicit OprTypeBase(ALLOC_INFO allocInfo):
            _allocInfo(allocInfo){}

            virtual ~OprTypeBase() = default;

            virtual bool isEqualTypeDeep(const OprTypeBase& rhs)  = 0;

            bool isEqualType(const OprTypeBase& rhs){
                bool prelimCheck = (_oprWidth  == rhs._oprWidth) &&
                                   (_oprType   == rhs._oprType)  &&
                                   (_subType   == rhs._subType)  &&
                                   (_allocInfo == rhs._allocInfo);
                if (!prelimCheck) {return false;}
                return isEqualTypeDeep(rhs);
            }

            std::string genOprFieldName(const std::string& fieldName,
                                        const std::string& description){
                return fieldName + "_" + description;
            }

            [[nodiscard]]
            int getOprWidth(){return _oprWidth;}

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

            OprMatcherBase(int instrWidth, OprTypeBase* oprType, const Slice& matchSl):
            SliceMatcher(instrWidth),
            _instrWidth(instrWidth),
            _oprType(oprType){
                crlAss(_instrWidth > 0, "instruction width must > 0");
                crlAss(_oprType != nullptr, "match oprType must not be null");
                addMatchedSlice(matchSl);
            }

        };

    }


#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_OPR_BASE_H
