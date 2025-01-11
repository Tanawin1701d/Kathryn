//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_fetchUnit_FETCHMETABASE_H
#define src_carolyne_arch_base_march_fetchUnit_FETCHMETABASE_H
#include <bitset>
#include "carolyne/arch/base/util/sliceMatcher.h"
#include "carolyne/util/viz/csvVizable.h"


    namespace kathryn::carolyne{

        ///// To support vary instruction length
        /// ExtendMatcher contain prefersize and the matchedslice
        /// to determine that it should be fetch more
        struct FetchExtendMatcher: SliceMatcher{
            int                   _preferSize = -1; ///// unit is bit size
            std::vector<uint64_t> _matchValues; ///// the match value that coherence with sliceMatcher match vector
            explicit FetchExtendMatcher(int preferSize):
                SliceMatcher(preferSize),
                _preferSize(preferSize){}

            std::vector<uint64_t> getMatchValue () const                                  {return _matchValues;}
            void                  addMatchValue (uint64_t matchValue)                     {_matchValues.push_back(matchValue);}
            void                  setMatchValues(const std::vector<uint64_t>& matchValues){_matchValues = matchValues;}
            std::string           getDebugMatchValue(){
                std::string result;
                for (int i= static_cast<int>(_matchValues.size()-1); i >= 0; i--){
                    std::string s = std::bitset<64>(_matchValues[i]).to_string(); // string conversion
                    result += s + "_";
                }
                return result;
            }
        };


        struct FetchUTM_Base: VizCsvGenable{

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

            void visual(CsvGenFile& genFile) override{

                char RN_PCWIDTH[]     = "pcWidth";
                char RM_ST_LD_WIDTH[] = "startInstrWidth";
                //////// table for main structure
                CsvTable table(1, 2);
                table.setHeadNames(true, {SRN_BITWIDTH});
                table.setHeadNames(false, {RN_PCWIDTH, RM_ST_LD_WIDTH});
                table.setData(SRN_BITWIDTH, RN_PCWIDTH    , std::to_string(_pcWidth));
                table.setData(SRN_BITWIDTH, RM_ST_LD_WIDTH, std::to_string(_startInstrWidth));
                genFile.addData(table);

                //////// table for fetch Extender

                char RN_PREFERSZ[] = "pcWidth";
                char RM_MATCHVAL[] = "startInstrWidth";

                CsvTable fetchExtendTable(2, static_cast<int>(_instrLengthIdents.size()));
                fetchExtendTable.setHeadNames(true, {RN_PREFERSZ, RM_MATCHVAL});
                fetchExtendTable.setsHeadNameIterative(false,"i");
                for (int colIdx = 0; colIdx < _instrLengthIdents.size(); colIdx++){
                    fetchExtendTable.setData(0, colIdx, std::to_string(_instrLengthIdents[colIdx]._preferSize));
                    fetchExtendTable.setData(1, colIdx, _instrLengthIdents[colIdx].getDebugMatchValue());
                }
                genFile.addData(fetchExtendTable);
            }

        };

    }


#endif //src_carolyne_arch_base_march_fetchUnit_FETCHMETABASE_H
