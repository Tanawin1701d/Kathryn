//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_alloc_ALLOCUNITMETABASE_H
#define src_carolyne_arch_base_march_alloc_ALLOCUNITMETABASE_H

#include "carolyne/arch/base/isa/regFile/archRegFile.h"
#include "carolyne/arch/base/march/robUnit/robMetaBase.h"
#include "carolyne/arch/base/march/rsvUnit/rsvMetaBase.h"

namespace kathryn::carolyne{




        ///// UTM  = unit type meta
        struct AllocUTM_Base: VizCsvGenable{

            std::vector<APRegTypeMatch> _regMatchers;
            std::vector<RsvUTM_Base*>   _rsvTypePtrs;
            RobUTM_Base*                _targetRob = nullptr; //// it is ok to make rob be nullptr

            void addRegFileMatcher(const std::string& archRegGrpName,
                                   const std::string& phyRegGrpName){
                _regMatchers.push_back({archRegGrpName, phyRegGrpName});
            }
            void addRsvType(RsvUTM_Base* rsv){
                crlAss(rsv != nullptr, "why add rsv to to nullptr");
                _rsvTypePtrs.push_back(rsv);
            }
            void setTargetRob(RobUTM_Base* rob){
                crlAss(rob != nullptr, "why set rob to nullptr? to alloc unit");
                _targetRob = rob;
            }
            [[nodiscard]]
            std::vector<APRegTypeMatch> getRegMatchers() const{return _regMatchers;}
            [[nodiscard]]
            std::vector<RsvUTM_Base*>   getRsvTypePtr () const{return _rsvTypePtrs;}
            [[nodiscard]]
            RobUTM_Base*                getRobTypeMeta() const{return _targetRob;}

            void visual(CsvGenFile& genFile) override{

                constexpr char RN_ARCH[] = "archRegFileName";
                constexpr char RN_PHY[]  = "phyRegFileName";
                crlAss(!_regMatchers.empty(), "in visualization regMatcher cannot empty()");
                CsvTable table(static_cast<int>(_regMatchers.size()), 2);
                table.setsHeadNameIterative(true, "archId");
                table.setHeadNames(false, {RN_ARCH, RN_PHY});
                int idx = 0;
                for (auto& matcher: _regMatchers){
                    table.setData(idx, 0, matcher._archRegGrpName);
                    table.setData(idx, 1, matcher._phyRegGrpName);
                    idx++;
                }
                table.setTableName("AllocUnitMeta");
                genFile.addData(table);

            }

        };



    }


#endif //src_carolyne_arch_base_march_alloc_ALLOCUNITMETABASE_H
