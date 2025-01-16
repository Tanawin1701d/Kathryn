//
// Created by tanawin on 30_12_2024.
//

#ifndef src_carolyne_arch_base_march_abstract_FIELDS_H
#define src_carolyne_arch_base_march_abstract_FIELDS_H

#include <algorithm>
#include<vector>
#include<map>
#include<string>
#include "lib/hw/slot/rowMeta.h"
#include "model/hwComponent/abstract/Slice.h"
#include "carolyne/util/checker/checker.h"
#include "lib/hw/slot/csvVizable.h"



namespace kathryn::carolyne{
        /**
         * RowMeta is abstract structure representing
         * the data slot and its meaning
         */


        enum CRL_GEN_MODE{
            CGM_FETCH,
            CGM_DECODE,
            CGM_ALLOC,
            CGM_RSV,
            CGM_EXEC,
            CGM_ROB
        };

        struct GenRowMetaable{
            virtual ~GenRowMetaable() = default;
            virtual RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) = 0;
            virtual RowMeta genRowMeta(const std::string& genMode) = 0;
        };

    }


#endif //src_carolyne_arch_base_march_abstract_FIELDS_H
