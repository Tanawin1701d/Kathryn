//
// Created by tanawin on 30/12/2024.
//

#ifndef src_carolyne_arch_base_util_SLICEMATCHER_H
#define src_carolyne_arch_base_util_SLICEMATCHER_H

#include <vector>
#include "carolyne/util/checker/checker.h"
#include "model/hwComponent/abstract/Slice.h"


    namespace kathryn::carolyne{
        /**
         * slice matcher is a collection of slice that used to match the region of interest on A hardware component
         */
        struct SliceMatcher{
            int                _limitSize = -1;
            std::vector<Slice> _matchedSlices; //// the slice in raw fetch instruction

            explicit SliceMatcher(int limitSize):
            _limitSize(limitSize){
                crlAss(_limitSize > 0, "slice matcher must have size > 0");
            }

            void addMatchedSlice(const Slice& sl){
                /** check system*/
                crlAss(sl.checkValidSlice(),
                           "invalid opr match slice " + std::to_string(sl.start) +
                           " " + std::to_string(sl.stop));
                crlAss(sl.stop <= _limitSize,
                       "sl.stop = " + std::to_string(sl.stop) +
                       "exceed instrwidth " + std::to_string(_limitSize));
                _matchedSlices.push_back(sl);
            }

            [[nodiscard]]
            std::vector<Slice> getMatchedSlices() const{return _matchedSlices;}
        };

    }




#endif //src_carolyne_arch_base_util_SLICEMATCHER_H
