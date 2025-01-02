//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_alloc_ALLOCUNITMETABASE_H
#define src_carolyne_arch_base_march_alloc_ALLOCUNITMETABASE_H

#include "carolyne/arch/base/isa/regFile/archRegFile.h"
#include "carolyne/arch/base/march/robUnit/robMetaBase.h"
#include "carolyne/arch/base/march/rsvUnit/rsvMetaBase.h"

namespace kathryn::carolyne{


        //////////// it map between architecture register group and
        //////////// physical register group
        struct ArchPhyRegTypeMatcher{
            std::string _archRegGrpName;
            std::string _phyRegGrpName;
        };


        struct AllocUnitTypeMeta{

            std::vector<ArchPhyRegTypeMatcher> _regMatchers;
            std::vector<RsvUnitTypeMeta*>      _rsvTypePtrs;
            RobTypeMeta*                       _targetRob = nullptr; //// it is ok to make rob be nullptr

            void addRegFileMatcher(const std::string& archRegGrpName,
                                   const std::string& phyRegGrpName){
                _regMatchers.push_back({archRegGrpName, phyRegGrpName});
            }
            void addRsvType(RsvUnitTypeMeta* rsv){
                crlAss(rsv != nullptr, "why add rsv to to nullptr");
                _rsvTypePtrs.push_back(rsv);
            }
            void setTargetRob(RobTypeMeta* rob){
                crlAss(rob != nullptr, "why set rob to nullptr? to alloc unit");
                _targetRob = rob;
            }
            [[nodiscard]]
            std::vector<ArchPhyRegTypeMatcher> getRegMatchers() const{return _regMatchers;}
            [[nodiscard]]
            std::vector<RsvUnitTypeMeta*>      getRsvTypePtr () const{return _rsvTypePtrs;}
            [[nodiscard]]
            RobTypeMeta*                       getRobTypeMeta() const{return _targetRob;}

        };



    }


#endif //src_carolyne_arch_base_march_alloc_ALLOCUNITMETABASE_H
