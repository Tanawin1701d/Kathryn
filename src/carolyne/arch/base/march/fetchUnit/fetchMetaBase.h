//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_fetchUnit_FETCHMETABASE_H
#define src_carolyne_arch_base_march_fetchUnit_FETCHMETABASE_H
#include "carolyne/arch/base/util/sliceMatcher.h"

namespace kathryn{
    namespace carolyne{

        ///// To support vary instruction length
        /// ExtendMatcher contain prefersize and the matchedslice
        /// to determine that it should be fetch more
        struct FetchExtendMatcher: SliceMatcher{
            int preferSize = -1;
            ExtendMatcher(int preferSize):
                SliceMatcher(preferSize),
                preferSize(preferSize){}
        };


        struct FetchTypeMeta{

            int _pcWidth = -1; ///// size in byte

            std::vector<FetchExtendMatcher> _instrLengthIdents;
            /// it is used to identify the size of the instruction and match to next size to fetch if the fetching is vary size
            /// it must sorted from low width to high width

            explicit FetchTypeMeta(int pcWidth): _pcWidth(pcWidth){}

            void addFetchExtendMatcher(const FetchExtendMatcher& fem){
                _instrLengthIdents.push_back(fem);
            }

            [[nodiscard]]
            std::vector<FetchExtendMatcher> getInstrLengthIdents() const{return _instrLengthIdents;}

        };

    }
}

#endif //src_carolyne_arch_base_march_fetchUnit_FETCHMETABASE_H
