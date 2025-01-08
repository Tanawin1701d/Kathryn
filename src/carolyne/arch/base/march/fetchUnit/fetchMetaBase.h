//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_fetchUnit_FETCHMETABASE_H
#define src_carolyne_arch_base_march_fetchUnit_FETCHMETABASE_H
#include "carolyne/arch/base/util/sliceMatcher.h"


    namespace kathryn::carolyne{

        ///// To support vary instruction length
        /// ExtendMatcher contain prefersize and the matchedslice
        /// to determine that it should be fetch more
        struct FetchExtendMatcher: SliceMatcher{
            int                   _preferSize = -1;
            std::vector<uint64_t> _matchValues; ///// the match value that coherence with sliceMatcher match vector
            explicit FetchExtendMatcher(int preferSize):
                SliceMatcher(preferSize),
                _preferSize(preferSize){}

            std::vector<uint64_t> getMatchValue () const                                  {return _matchValues;}
            void                  addMatchValue (uint64_t matchValue)                     {_matchValues.push_back(matchValue);}
            void                  setMatchValues(const std::vector<uint64_t>& matchValues){_matchValues = matchValues;}
        };


        struct FetchUTM_Base{

            int _pcWidth         = -1; ///// size in byte
            int _startInstrWidth = -1;

            std::vector<FetchExtendMatcher> _instrLengthIdents;
            /// it is used to identify the size of the instruction and match to next size to fetch if the fetching is vary size
            /// it must sorted from low width to high width

            explicit FetchUTM_Base(int pcWidth, int startInstrWidth):
            _pcWidth(pcWidth),
            _startInstrWidth(startInstrWidth){}

            void addFetchExtendMatcher(const FetchExtendMatcher& fem){
                _instrLengthIdents.push_back(fem);
            }

            [[nodiscard]]
            std::vector<FetchExtendMatcher> getInstrLengthIdents() const{return _instrLengthIdents;}

        };

    }


#endif //src_carolyne_arch_base_march_fetchUnit_FETCHMETABASE_H
